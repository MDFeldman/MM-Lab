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

    printf("AHHH IM GONNA CHECK THE HEAP\n");

    //ensure prev of head is NULL
    if (cur && cur->prev) {
        printf("RET -5\n");
        return -5;
    }

    while (cur) {
        printf("CHECKING HEAP CURR %p\n", cur);
        printf("CHECKING HEAP CURR SIZE %016lX\n", get_size(cur));
        printf("CHECKING HEAP CURR NEXT %p\n", cur->next);
        printf("CHECKING HEAP CURR PREV %p\n", cur->prev);

        if (is_allocated(cur)) {
            printf("RET -1\n");
            return -1;
        }
        size_t pos = (size_t) cur;

        if (pos % ALIGNMENT != 0) {
            printf("RET -2\n");
            return -2;
        }
        if (cur->next) {
            size_t nextPos = (size_t) cur->next;
            size_t size = get_size(cur);
/*
            printf("DATA A %p\n", cur);
            printf("DATA B %016lX\n", size);
            printf("DATA C %016lX\n", sizeof(memory_block_t));
            printf("DATA D %p\n", cur->next);
            printf("DATA E %016lX\n",  ((size_t)cur->next) - ((size_t) cur));
*/
            if (pos + size + sizeof(memory_block_t) > nextPos) {
                printf("RET -3\n");
                return -3;
            }
            if (cur->next->prev != cur) {
                printf("CUR NEXT %p\n", cur->next);
                printf("CUR N->P %p\n", cur->next->prev);
                printf("RET -4\n");
                return -4;
            }
        }
        cur = cur->next;
    }

    printf("RET 0\n");
    return 0;
}