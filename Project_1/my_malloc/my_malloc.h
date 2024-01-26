#ifndef __MY_MALLOC__
#define __MY_MALLOC__
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

struct _block_meta {
    size_t dataSize;
    bool allocated;
    struct _block_meta * prev;
    struct _block_meta * next;
};
typedef struct _block_meta block_meta;

#define META_SIZE sizeof(block_meta)

/*
 * @brief Global variables to track heap information.
 */
struct _heap_info_t {
    size_t totalAllocated;
    size_t totalFreed;
};
typedef struct _heap_info_t heap_info_t;

/*
 * @brief Initializes block metadata.
 * @param block: Pointer to the block metadata.
 * @param dataSize: Size of the data in the block.
 * @param allocated: Allocation status of the block.
 */
void initBlockMeta (block_meta * block, size_t dataSize, bool allocated);

/*
 * @brief Displays metadata information of a block.
 *
 * @param block: Pointer to the block metadata.
 */
void displayBlockMeta(block_meta * block);


struct _linkedlist {
    block_meta * head;
    block_meta * tail;
};
typedef struct _linkedlist LinkedList;

/*
 * @brief Checks if a linked list is empty.
 *
 * @param list: Pointer to the linked list.
 * @return Boolean indicating whether the list is empty.
 */
bool isEmpty(LinkedList * list);

/*
 * @brief Appends a block to the free list.
 * @param list: Pointer to the linked list.
 * @param toAppend: Pointer to the block to be appended.
 */
void appendToFreeList(LinkedList * list, block_meta * toAppend);

/*
 * @brief Removes a block from the free list.
 * @param list: Pointer to the linked list.
 * @param toRemove: Pointer to the block to be removed.
 */
void removeFromFreeList(LinkedList * list, block_meta * toRemove);

/*
 * @brief Inserts a block in front of another block in the free list.
 * @param list: Pointer to the linked list.
 * @param toInsert: Pointer to the block to be inserted.
 * @param curr: Pointer to the block in front of which to insert.
 */
void insertInFrontOf(LinkedList * list, block_meta * toInsert, block_meta * curr);

/*
 * @brief Prints the free list for debugging purposes.
 * @param toPrint: Pointer to the linked list to be printed.
 */
void printFreeList(LinkedList * toPrint);

/*
 * @brief Splits a block to allocate the required size.
 * @param block: Pointer to the block to be split.
 * @param size: Size of the data needed.
 * @return Pointer to the allocated block.
 */
void * splitBlock(block_meta * block_, size_t size);

/*
 * @brief Allocates memory.
 * @param dataSize: Size of the data to be allocated.
 * @return Pointer to the allocated memory.
 */
void * allocate (size_t dataSize);

/*
 * @brief Deallocates memory.
 * @param toDeallocate: Pointer to the memory block to be deallocated.
 */
void deallocate (block_meta * toDeallocate);

/*
 * @brief Coalesces with the left adjacent block.
 * @param rightBlock: Pointer to the block to the right.
 */
void coalesceWithLeft (block_meta * rightBlock);

/*
 * @brief Coalesces with the right adjacent block.
 * @param leftBlock: Pointer to the block to the left.
 */
void coalesceWithRight(block_meta * leftBlock);

/*
 * @brief Finds the first fit block in the free list.
 * @param curr: Pointer to the current block in the free list.
 * @param size: Size of the data needed.
 * @return Pointer to the first fit block.
 */
block_meta * findFirstFit(block_meta * curr, size_t size);

/*
 * @brief Finds the best fit block in the free list.
 * @param curr: Pointer to the current block in the free list.
 * @param size: Size of the data needed.
 * @return Pointer to the best fit block.
 */
block_meta * findBestFit(block_meta * curr, size_t size);

/*
 * @brief First-fit memory allocation.
 * @param size: Size of the data needed.
 * @return Pointer to the allocated memory.
 */
void * ff_malloc (size_t size);

/*
 * @brief First-fit memory deallocation.
 * @param toFree: Pointer to the memory block to be deallocated.
 */
void ff_free (void * toFree);

/*
 * @brief Best-fit memory allocation.
 * @param size: Size of the data needed.
 * @return Pointer to the allocated memory.
 */
void * bf_malloc (size_t size);

/*
 * @brief Best-fit memory deallocation.
 * @param toFree: Pointer to the memory block to be deallocated.
 */
void  bf_free (block_meta * toFree);

/*
 * @brief Gets the total size of the data segment.
 * @return Total size of the data segment.
 */
unsigned long get_data_segment_size();

/*
 * @brief Gets the free space size in the data segment.
 * @return Free space size in the data segment.
 */
unsigned long get_data_segment_free_space_size();

#endif