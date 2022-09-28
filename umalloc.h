#include <stdlib.h>
#include <stdbool.h>

#define ALIGNMENT 16 /* The alignment of all payloads returned by umalloc */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/*
 * memory_block_t - Represents a block of memory managed by the heap. The 
 * struct can be left as is, or modified for your design.
 * In the current design bit0 is the allocated bit
 * bits 1-3 are unused.
 * and the remaining 60 bit represent the size.
 */
typedef struct memory_block_struct {
    size_t block_size_alloc;
    struct memory_block_struct *next;
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
    @Description: return the size of the payload of a given memory_block_t struct
*/
size_t get_size(memory_block_t *block);
/*
    @Description: find the proceeding block that is pointed to in the linked list by a given memory_block_t
*/
memory_block_t *get_next(memory_block_t *block);
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
    @Description: return a memory_block_t of sufficient size to satisfy a malloc request of a given size
*/
memory_block_t *find(size_t size);
/*
    @Description: In the event that more memory is needed to satisfy malloc requests
        extend() may be called to increase the size of the heap
*/
memory_block_t *extend(size_t size);
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