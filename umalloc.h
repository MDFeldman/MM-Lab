#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ALIGNMENT 16 /* The alignment of all payloads returned by umalloc */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

#define SPLIT_THRESHOLD 64 /* The amount of extra free space required to warrant splitting a free block */

/*
 * memory_block_t - Represents a block of memory managed by the heap. The 
 * struct can be left as is, or modified for your design.
 * In the current design bit0 is the allocated bit
 * bit1 is set whenever there is a contiguously adjacent preceeding block (does not specifiy if it is allocated or not)
 * bit2 is set whenever there is a contiguously adjacent proceeding block (does not specifiy if it is allocated or not)
 * bit3 is unused.
 * and the remaining 60 bit represent the size.
 */
typedef struct memory_block_struct {
    size_t block_size_alloc;
    //next block whether that be allocated or free
    struct memory_block_struct *prev;
    struct memory_block_struct *next;
    struct memory_block_struct *prev_adjacent;
} memory_block_t;

// Helper Functions, this may be editted if you change the signature in umalloc.c

/*
*  STUDENT TODO:
*      Write 1-2 sentences for each function explaining what it does. Don't just repeat the name of the function back to us.
*/

/*
    @Description: returns whether or not a block is in use by the mutator
        returns true if in use
        returns false if free
*/
bool is_allocated(memory_block_t *block);

/*
    @Description: returns if a memory_block_t has a contiguously adjacent preceeding block
*/
bool has_preceeding(memory_block_t *block);
/*
    @Description: returns if a memory_block_t has a contiguously adjacent proceeding block
*/
bool has_proceeding(memory_block_t *block);

/*
    @Description: returns the preceeding contiguously adjascent block to a given block
*/
memory_block_t *get_preceeding(memory_block_t *block);
/*
    @Description: returns the proceeding contiguously adjascent block to a given block
*/
memory_block_t *get_proceeding(memory_block_t *block);


/*
    @Description: set a memory block to the status of being allocated
        when a free block is to be put into use call this and pass said block
*/
void allocate(memory_block_t *block);
/*
    @Description: set a memory block to the status of being free
        when an allocated block is to be taken out of use pass said block to this function
*/
void deallocate(memory_block_t *block);


/*
    @Description: records within block_size_alloc that a given memory block has a contiguously adjacent preceeding block
*/
void set_exists_preceeding(memory_block_t *block);
/*
    @Description: records within block_size_alloc that a given memory block has no contiguously adjacent preceeding block
*/
void set_no_preceeding(memory_block_t *block);
/*
    @Description: records within block_size_alloc that a given memory block has a contiguously adjacent proceeding block
*/
void set_exists_proceeding(memory_block_t *block);
/*
    @Description: records within block_size_alloc that a given memory block has no contiguously adjacent proceeding block
*/
void set_no_proceeding(memory_block_t *block);

/*
    @Description: return the size of the payload of a given memory_block_t struct
*/
size_t get_size(memory_block_t *block);
/*
    @Description: find the proceeding block that is pointed to in the linked list by a given memory_block_t
*/
memory_block_t *get_next(memory_block_t *block);
/*
    @Description: find the preceeding block that is pointed to in the linked list by a given memory_block_t
*/
memory_block_t *get_prev(memory_block_t *block);
/*
    @Description: initialize a new memory_block_t at a given address,
        initialize it with (size, alloc) and set the next block pointer appropriately
*/
void put_block(memory_block_t *block, size_t size, bool alloc);
/*
    @Description: return a pointer to the "payload" or contents of a given memory_block_t
        in other words, return a pointer to the data that the mutator is/would be using if said block is allocated
*/
void *get_payload(memory_block_t *block);
/*
    @Description: given a pointer to a piece of data that may or may not have been delegated to the mutator,
        return a pointer to the corresponding memory_block_t struct
*/
memory_block_t *get_block(void *payload);


/*
    @Description: get the minimum size of some payload + an object when padded
*/
size_t get_min_padded_size(size_t payload_size, size_t type_size);
/*
    @Description: get the entire size of a block, meaning both the size of the header and payload combined
*/
size_t get_entire_size(memory_block_t * block);
/*
    @Description: add a free block to the free list without any hints as to where it may fit
*/
void insert_free_block_no_context(memory_block_t *block);
/*
    @Description: add a block into the free block list, provide a hint as to the previous free block
*/
void insert_free_block_hint(memory_block_t *new_free, memory_block_t *hint);

/*
    @Description: return a memory_block_t of sufficient size to satisfy a malloc request of a given size
*/
memory_block_t *find(size_t size);
/*
    @Description: In the event that more memory is needed to satisfy malloc requests
        extend() may be called to increase the size of the heap,
        this version finds last sbrk_block_t in list and calls extend_pass_sbrk()
*/
memory_block_t *extend(size_t size);

/*
    @Description: In the event that more memory is needed to satisfy malloc requests
        extend_hint() may be called to increase the size of the heap,
        this is another version of extend() that takes in a hint as to where the newly allocated memory will go in the free list
        good for optimizaiton
*/
memory_block_t *extend_hint(size_t size, memory_block_t * hint);

/*
    @Description: divide a free block into two sections, one allocated block, and the remaining space free
*/
memory_block_t *split(memory_block_t *block, size_t size);
/*
    @Description: combine a free block with adjacent free blocks to create one larger free block
*/
memory_block_t *coalesce(memory_block_t *block);


// Portion that may not be edited
int uinit();
void *umalloc(size_t size);
void ufree(void *ptr);