#include <stdio.h>
#include "umalloc.h"
#include "csbrk.h"

//Place any variables needed here from umalloc.c or csbrk.c as an extern.
extern memory_block_t *free_head;
extern sbrk_block *sbrk_blocks;

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 
 * STUDENT TODO:
 * Required to be completed for checkpoint 1:
 *      - Check that pointers in the free list point to valid free blocks. Blocks should be within the valid heap addresses: look at csbrk.h for some clues.
 *        They should also be allocated as free.
 *      - Check if any memory_blocks (free and allocated) overlap with each other. Hint: Run through the heap sequentially and check that
 *        for some memory_block n, memory_block n+1 has a sensible block_size and is within the valid heap addresses.
 *      - Ensure that each memory_block is aligned. 
 * 
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {
    // Example heap check:
    // Check that all blocks in the free list are marked free.
    // If a block is marked allocated, return -1.
    memory_block_t *cur = free_head;

    //ensure prev of head is NULL
    if (cur && cur->prev) {
        return -1;
    }

    /*
        Loop through the list to ensure every free block has no issues
    */
    while (cur) {
        /*
            Ensure every free block is unallocated and aligned
        */
        if (is_allocated(cur)) {
            return -2;
        }
        size_t pos = (size_t) cur;
        if (pos % ALIGNMENT != 0) {
            return -3;
        }

        /*
            Ensure that cur->next points back to cur with ->prev and that there is no overlap between the blocks
        */

        if (cur->next) {
            size_t nextPos = (size_t) cur->next;
            size_t size = get_size(cur);
            if (pos + size + sizeof(memory_block_t) > nextPos) {
                return -4;
            }
            if (cur->next->prev != cur) {
                return -5;
            }
        }

        /*
            Ensure that cur's adjacent contiguous blocks link back to cur
        */

        if(has_preceeding(cur)) {
            memory_block_t * preceeding = get_preceeding(cur);
            if (!preceeding) {
                return -6;
            }
            if (get_proceeding(preceeding) != cur) {
                return -7;
            }
        }
        if (has_proceeding(cur)) {
            memory_block_t * proceeding = get_proceeding(cur);
            if (!proceeding) {
                return -8;
            }
            if (get_preceeding(proceeding) != cur) {
                return -9;
            }
        }
        cur = cur->next;
    }

    return 0;
}