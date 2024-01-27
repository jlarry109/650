#ifndef __MY_MALLOC__
#define __MY_MALLOC__
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

struct MemoryBlock {
  size_t dataSize;
  bool allocated;
  struct MemoryBlock* prev;
  struct MemoryBlock* next;
};
typedef struct MemoryBlock MemoryBlock;

/*
 * @brief Linked list structure to manage free blocks in the heap.
 */
struct FreeList {
  MemoryBlock* head;
  MemoryBlock* tail;
};
typedef struct FreeList FreeList;

#define META_SIZE sizeof(MemoryBlock)

/*
 * @brief Global variables to track heap information.
 */
struct _heap_info_t {
    size_t totalAllocated;
    size_t totalFreed;
};
typedef struct _heap_info_t heap_info_t;

/*
 * @brief Checks if a linked list is empty.
 * @param list: Pointer to the linked list.
 * @return Boolean indicating whether the list is empty.
 */
bool isEmptyFreeList (FreeList * freeList);


/*
 * @brief Initializes block metadata.
 * @param block: Pointer to the block metadata.
 * @param dataSize: Size of the data in the block.
 * @param allocated: Allocation status of the block.
 */
void initializeMemoryBlock(MemoryBlock* block, size_t dataSize, bool occupied);


/*
 * @brief Appends a block to the free list.
 * @param list: Pointer to the linked list.
 * @param toAppend: Pointer to the block to be appended.
 */
void appendToFreeList(FreeList* list, MemoryBlock* block);

/*
 * This function searches for the first MemoryBlock in the linked list starting
 * from the given 'curr' block that has enough space to accommodate the specified 'size'.
 * The search is performed sequentially, and the first fitting block is returned.
 * If no suitable block is found, NULL is returned.
 *
 * @param curr  Pointer to the starting MemoryBlock in the linked list.
 *              The search begins from this block.
 * @param size  The size of the memory space required.
 * @return      Pointer to the first MemoryBlock that fits the specified size,
 *              or NULL if no suitable block is found.
 */
MemoryBlock * findFirstFit(MemoryBlock * curr, size_t size);

/*
 * This function inserts the specified MemoryBlock into the given FreeList.
 * If the FreeList is empty, the inserted block becomes both the head and tail
 * of the list. If 'curr' is NULL, the block is inserted at the beginning of the list.
 * If 'curr' is the tail of the list, the block is inserted at the end.
 * Otherwise, the block is inserted between 'curr' and 'curr->next'.

 * @param list  Pointer to the FreeList where the block should be inserted.
 * @param block Pointer to the MemoryBlock to be inserted into the FreeList.
 * @param curr  Pointer to the current MemoryBlock in the FreeList after which
 *              the new block should be inserted. If NULL, the block is inserted
 *              at the beginning.
 */
void insertIntoFreeList(FreeList *list, MemoryBlock *block, MemoryBlock * curr);


/*
 * @brief Removes a block from the free list.
 * @param list: Pointer to the linked list.
 * @param toRemove: Pointer to the block to be removed.
 */
void removeFromFreeList(FreeList* list, MemoryBlock* toRemove);

/*
 * @brief Allocates memory.
 * @param dataSize: Size of the data to be allocated.
 * @return Pointer to the allocated memory.
 */
void* allocateMemory(size_t dataSize);
/*
 * @brief Splits a block to allocate the required size.
 * @param block: Pointer to the block to be split.
 * @param size: Size of the data needed.
 * @return Pointer to the allocated block.
 */
MemoryBlock* splitMemoryBlock(MemoryBlock* block, size_t dataSize);

/*
 * @brief Coalesces with the left adjacent block.
 * @param rightBlock: Pointer to the block to the right.
 */
void coalesceWithLeft(MemoryBlock* block);

/*
 * @brief Coalesces with the right adjacent block.
 * @param leftBlock: Pointer to the block to the left.
 */
void coalesceWithRight(MemoryBlock* block);


/*
 * Frees a MemoryBlock and updates the free list, performing coalescing if needed.

 * This function marks the specified MemoryBlock as unallocated, updates the total
 * freed memory in the heap_info structure, and adds the block to the free list. If
 * the free list is empty or the block is at the end of the free list, the block is
 * appended to the list. If the block is at the beginning, it is inserted at the start.
 * If the block is in the middle, it is inserted after the suitable block in the list.
 * Coalescing with neighboring free blocks is performed to merge contiguous free blocks.
 *
 * @param block Pointer to the MemoryBlock to be freed.
 */
void freeMemoryBlock(MemoryBlock* block);

/*
 * @brief First-fit memory allocation.
 * @param size: Size of the data needed.
 * @return Pointer to the allocated memory.
 */
void* ff_malloc(size_t size);

/*
 * @brief First-fit memory deallocation.
 * @param toFree: Pointer to the memory block to be deallocated.
 */
void ff_free(void* ptr);

/*
 * @brief Best-fit memory allocation.
 * @param size: Size of the data needed.
 * @return Pointer to the allocated memory.
 */
void* bf_malloc(size_t size);

/*
 * @brief Best-fit memory deallocation.
 * @param toFree: Pointer to the memory block to be deallocated.
 */
void bf_free(void* ptr);

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