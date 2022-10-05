#include "umalloc.h"
#include "csbrk.h"
#include "ansicolors.h"
#include <stdio.h>
#include <assert.h>

const char author[] = ANSI_BOLD ANSI_COLOR_RED "MAX FELDMAN:mdf2627" ANSI_RESET;

/*
 * The following helpers can be used to interact with the memory_block_t
 * struct, they can be adjusted as necessary.
 */

// A sample pointer to the start of the free list.
memory_block_t *free_head;

/*
 * is_allocated - returns true if a block is marked as allocated.
 */
bool is_allocated(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & 0x1;
}

bool has_preceeding(memory_block_t *block) {
    assert(block != NULL);
    return (block->block_size_alloc>>1) & 0x1;
}

bool has_proceeding(memory_block_t *block) {
    assert(block != NULL);
    return (block->block_size_alloc>>2) & 0x1;
}

memory_block_t *get_preceeding(memory_block_t *block) {
    if (!has_preceeding(block)) {
        return NULL;
    }
    return block->prev_adjacent;
}

memory_block_t *get_proceeding(memory_block_t *block) {
    if (!has_proceeding(block)) {
        return NULL;
    }
    void* ptr = get_payload(block) + get_size(block);
    return (memory_block_t *) ptr;
}

/*
 * allocate - marks a block as allocated.
 */
void allocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc |= 0x1;
}

/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x1;
}

void set_exists_preceeding(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc |= 0x2;
}

void set_no_preceeding(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x2;
}

void set_exists_proceeding(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc |= 0x4;
}

void set_no_proceeding(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x4;
}

void set_size(memory_block_t *block, size_t size) {
    assert(block != NULL);
    block->block_size_alloc &= ALIGNMENT-1;
    block->block_size_alloc = (size & ~(ALIGNMENT-1)) | block->block_size_alloc;
}

/*
 * get_size - gets the size of the block.
 */
size_t get_size(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & ~(ALIGNMENT-1);
}

/*
 * get_next - gets the next block.
 */
memory_block_t *get_next(memory_block_t *block) {
    assert(block != NULL);
    return block->next;
}

/*
 * get_prev - gets the previous block.
 */
memory_block_t *get_prev(memory_block_t *block) {
    assert(block != NULL);
    return block->prev;
}

/*
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with NUlling out the next 
 * field.
 */
void put_block(memory_block_t *block, size_t size, bool alloc) {
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->prev = NULL;
    block->next = NULL;
    block->prev_adjacent = NULL;
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    return (void*)(block + 1);
}

/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    return ((memory_block_t *)payload) - 1;
}

/*
 *  STUDENT TODO:
 *      Describe how you select which free block to allocate. What placement strategy are you using?
 *
 *
 *      I allocate the first free block which fits the description
 */

size_t get_min_padded_size(size_t payload_size, size_t type_size) {
    size_t sans_pad = payload_size + type_size;
    return sans_pad + ((ALIGNMENT - (sans_pad % ALIGNMENT)) % ALIGNMENT);
}

size_t get_entire_size(memory_block_t * block) {
    assert(block);
    return get_min_padded_size(get_size(block), sizeof(memory_block_t));
}

void check_adjacent(memory_block_t *block, bool st, bool print) {
    memory_block_t * temp0 = block;
    if (print) printf("SPLIT LISTING PROCEEDINGS\n");
    bool A = true;
    while (has_proceeding(temp0)) {
        memory_block_t *next = get_proceeding(temp0);
        if (print) printf("\tTEMP 0:next %p:%p:%d\n", temp0, next, is_allocated(temp0));
        A &= (temp0 == get_preceeding(next));
        if (print) printf("\t\t%p:%d\n", get_preceeding(next), has_preceeding(next));
        temp0 = next;
    }


    temp0 = block;
//    temp0 = has_preceeding(temp0) ? get_preceeding(temp0) : temp0;
    if (print) printf("SPLIT LISTING PRECEEDINGS\n");
    while(has_preceeding(temp0)) {
        memory_block_t *next = get_preceeding(temp0);
        if (print) printf("\tTEMP 0:next %p:%p:%d\n", temp0, next, is_allocated(temp0));
        A &= (temp0 == get_proceeding(next));
        if (print) printf("\t\t%p:%d\n", get_proceeding(next), has_proceeding(next));
        temp0 = next;
    }
    assert(!st || A);
}

void check_all(bool st, bool print) {
    memory_block_t * cur = free_head;
    while(cur) {
        check_adjacent(cur, st, print);
        cur = cur->next;
    }
}

void insert_free_block_no_context(memory_block_t *new_free) {
    memory_block_t *cur = free_head;

    if (!cur) {
        free_head = new_free;
        free_head->prev = NULL;
        free_head->next = NULL;
        return;
    }

    if (cur > new_free) {
        free_head = new_free;
        cur->prev = new_free;
        free_head->next = cur;
        free_head->prev = NULL;
        return;
    }
    while (cur < new_free && cur->next) cur = cur->next;

    if (cur >= new_free) {
        assert(cur != new_free);
        new_free->next = cur;
        new_free->prev = cur->prev;
        if (cur->prev) {
            cur->prev->next = new_free;
        }
        cur->prev = new_free;
    }
    else { // !cur->next
        cur->next = new_free;
        new_free->prev = cur;
    }
    return;
}


/*
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size) {
    //? STUDENT TODO
    memory_block_t * cur = free_head;
    while (cur) {
        size_t min_padded_size = get_min_padded_size(size, 0);
        size_t payload_size = get_size(cur);
        if (payload_size >= min_padded_size) {
            if (payload_size >= min_padded_size + SPLIT_THRESHOLD) {
                split(cur, min_padded_size);
            }
            else {
                if (cur->prev) {
                    cur->prev->next = cur->next;
                }
                if (cur->next) {
                    cur->next->prev = cur->prev;
                }
                allocate(cur);
            }
            if (cur == free_head) {
                free_head = cur->next;
            }
            return cur;
        }
        cur = cur->next;
    }
    memory_block_t *ext = extend(size);
    if (!ext) {
        return NULL;
    }
    split(ext, size);
    return ext;
}

/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size) {
    //? STUDENT TODO
    size_t DEFAULT_SIZE = PAGESIZE * 4;
    size_t request = size > DEFAULT_SIZE - sizeof(memory_block_t) ? ((PAGESIZE - (size % PAGESIZE)) % PAGESIZE) + size + PAGESIZE : DEFAULT_SIZE;
    void * new_heap = csbrk(request);

    memory_block_t *new_free = new_heap;
    size_t payload_size = request - get_min_padded_size(0, sizeof(memory_block_t));

    put_block(new_free, payload_size, false);
    set_no_preceeding(new_free);
    set_no_proceeding(new_free);
    //new_free->prev_adjacent = NULL

    insert_free_block_no_context(new_free);

    return new_free;
}


/*
 *  STUDENT TODO:
 *      Describe how you chose to split allocated blocks. Always? Sometimes? Never? Which end?
 *
 *  Allocated blocks will be split whenever the amount of excess space is above a threshold SPLIT_THRESHOLD
*/

/*
 * split - splits a given block in parts, one allocated, one free.
 *
 * @Return: the new free block
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    //? STUDENT TODO
    assert(!is_allocated(block));
    size_t original_size = get_size(block);
    size_t total_space = get_entire_size(block);
    size_t min_padded_size = get_min_padded_size(size, sizeof(memory_block_t));
    size_t min_padded_payload = min_padded_size - sizeof(memory_block_t);
    size_t free_block_alloc = total_space - min_padded_size - sizeof(memory_block_t);

    if (min_padded_size + sizeof(memory_block_t) + SPLIT_THRESHOLD >= original_size) {
        assert(total_space >= min_padded_size);
        allocate(block);
        if (block == free_head) {
            free_head = block->next;
        }
        if (block->prev) {
            block->prev->next = block->next;
        }
        if (block->next) {
            block->next->prev = block->prev;
        }

        return block;
    }
    memory_block_t * free = ((void*) block) + min_padded_size;

    put_block(free, free_block_alloc, false);
    free->next = block->next;
    free->prev = block->prev;
    free->prev_adjacent = block;
    if (block == free_head) {
        free_head = free;
    }
    if (free->next) {
        free->next->prev = free;
    }
    if (free->prev) {
        free->prev->next = free;
    }

    if (has_proceeding(block)) {
        set_exists_proceeding(free);
        get_proceeding(free)->prev_adjacent = free;
    }
    else {
        set_no_proceeding(free);
    }
    set_exists_proceeding(block);
    set_exists_preceeding(free);

    block->next = free;
    allocate(block);
    set_size(block, min_padded_payload);

    return free;
}

/*
 * coalesce - coalesces a free memory block with neighbors.
 */
memory_block_t *coalesce(memory_block_t *block) {
    //? STUDENT TODO
    assert(block);
    assert(!is_allocated(block));
    memory_block_t * write_to = block;
    memory_block_t * last = block;
    size_t new_size;

    memory_block_t * preceeding = get_preceeding(block);
    memory_block_t * proceeding = get_proceeding(block);

    if (preceeding && !is_allocated(preceeding)) {
        write_to = preceeding;
        new_size = get_entire_size(block) + get_size(preceeding);
    }
    else {
        new_size = get_size(block);
    }

    if (proceeding && !is_allocated(proceeding)) {
        new_size += get_entire_size(proceeding);
        last = proceeding;
    }

    if (write_to == last) {
        insert_free_block_no_context(block);

        return block;
    }

    if (!free_head) {
        free_head = write_to;
        free_head->next = NULL;
        free_head->prev = NULL;
        return write_to;
    }

    memory_block_t * lt_previously_free = write_to == block ? last : write_to;
    memory_block_t * rt_previously_free = last == block ? write_to : last;
    assert(lt_previously_free);
    assert(rt_previously_free);

    if (write_to < free_head) {
        assert(free_head == last);
        free_head = write_to;
    }

    set_size(write_to, new_size);
    write_to->prev = lt_previously_free->prev;
    if (write_to->prev) {
        write_to->prev->next = write_to;
    }
    write_to->next = rt_previously_free->next;
    if (write_to->next) {
        write_to->next->prev = write_to;
    }

    // write_to->prev_adjacent = write_to->prev_adjacent
    // set has_preceeding(write_to) to has_preceeding(write_to)
    // set has_proceeding(write_to) to has_proceeding(last)
    if (has_proceeding(last)) {
        set_exists_proceeding(write_to);
        memory_block_t *last_proceeding = get_proceeding(last);
        assert(is_allocated(last_proceeding));

        assert(has_preceeding(last_proceeding));
        last_proceeding->prev_adjacent = write_to;
    }
    else {
        set_no_proceeding(write_to);
    }
    return write_to;
}



/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    //* STUDENT TODO
    size_t request = PAGESIZE << 3;
    free_head = csbrk(request);
    size_t payload_size = request - get_min_padded_size(0, sizeof(memory_block_t));
    put_block(free_head, payload_size, false);
    set_no_preceeding(free_head);
    set_no_proceeding(free_head);
    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    //* STUDENT TODO
    memory_block_t * block = find(size);
    if (block) {
        return get_payload(block);
    }
    return NULL;
}

/*
 *  STUDENT TODO:
 *      Describe your free block insertion policy.
*/

/*
 * ufree -  frees the memory space pointed to by ptr, which must have been called
 * by a previous call to malloc.
 */
void ufree(void *ptr) {
    //* STUDENT TODO

    memory_block_t * new_free = get_block(ptr);

    assert(is_allocated(new_free));
    deallocate(new_free);

    coalesce(new_free);

//    insert_free_block_no_context(new_free);
}