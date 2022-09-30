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
 */

size_t get_min_padded_size(size_t payload_size, size_t type_size) {
    size_t sans_pad = payload_size + type_size;
    return sans_pad + ((ALIGNMENT - (sans_pad % ALIGNMENT)) % ALIGNMENT);
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
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size) {
    //? STUDENT TODO
    size_t DEFAULT_SIZE = PAGESIZE * 4;
    size_t request = size > DEFAULT_SIZE ? ((PAGESIZE - (size % PAGESIZE)) % PAGESIZE) + size + PAGESIZE : DEFAULT_SIZE;
    void * new_heap = csbrk(request);
    memory_block_t *new_free = new_heap;

    put_block(new_free, request - get_min_padded_size(0, sizeof(memory_block_t)), false);
    set_no_preceeding(new_free);
    set_no_proceeding(new_free);

    memory_block_t *cur = free_head;
    if (cur > new_free) {
        free_head = new_free;
        cur->prev = new_free;
        free_head->next = cur;
        free_head->prev = NULL;
    }
    while (cur < new_free && cur->next) cur = cur->next;

    if (cur >= new_free) {
        assert(cur != new_free);
        new_free->next = cur;
        new_free->prev = cur->prev;
        cur->prev = new_free;
    }
    else { // !cur->next
        cur->next = new_free;
        new_free->prev = cur;
    }

    return new_free;
}


/*
 *  STUDENT TODO:
 *      Describe how you chose to split allocated blocks. Always? Sometimes? Never? Which end?

    Allocated blocks will be split whenever the amount of excess space is above a threshold SPLIT_THRESHOLD
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
    size_t total_space = sizeof(memory_block_t) + original_size;
    size_t min_padded_size = get_min_padded_size(size, sizeof(memory_block_t));
    size_t free_block_alloc = total_space - min_padded_size - sizeof(memory_block_t);

    if (min_padded_size + sizeof(memory_block_t) + SPLIT_THRESHOLD >= original_size) {
        assert(total_space >= min_padded_size);
        return block;
    }
    memory_block_t * free = ((void*) block) + min_padded_size;
    put_block(free, free_block_alloc, false);
    free->next = block->next;
    free->prev = block->prev;
    if (free->next) {
        free->next->prev = free;
    }
    if (free->prev) {
        free->prev->next = free;
    }

    if (has_proceeding(block)) {
        set_exists_proceeding(free);
    }
    else {
        set_no_proceeding(free);
    }
    set_exists_proceeding(block);
    set_exists_preceeding(free);

    block->block_size_alloc = min_padded_size;
    block->next = free;
    allocate(block);
    return free;
}

/*
 * coalesce - coalesces a free memory block with neighbors.
 */
memory_block_t *coalesce(memory_block_t *block) {
    //? STUDENT TODO
    return NULL;
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
    deallocate(new_free);
    memory_block_t *cur = free_head;

    if (cur > new_free) {
        free_head = new_free;
        cur->prev = new_free;
        free_head->next = cur;
        free_head->prev = NULL;
    }
    while (cur < new_free && cur->next) cur = cur->next;

    if (cur >= new_free) {
        assert(cur != new_free);
        new_free->next = cur;
        new_free->prev = cur->prev;
        cur->prev = new_free;
    }
    else { // !cur->next
        cur->next = new_free;
        new_free->prev = cur;
    }

}