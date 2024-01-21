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

struct _heap_info_t {
    size_t totalAllocated;
    size_t totalFreed;
};
typedef struct _heap_info_t heap_info_t;

void initBlockMeta (block_meta * block, size_t dataSize, bool allocated);
void displayBlockMeta(block_meta * block);


struct _linkedlist {
    block_meta * head;
    block_meta * tail;
};
typedef struct _linkedlist LinkedList;

bool isEmpty(LinkedList * list);
void appendToFreeList(LinkedList * list, block_meta * toAppend);
void removeFromFreeList(LinkedList * list, block_meta * toRemove);
void insertInFrontOf(LinkedList * list, block_meta * toInsert, block_meta * curr);
void printFreeList(LinkedList * toPrint);

void * splitBlock(block_meta * block_, size_t size);

void * allocate (size_t dataSize);
void * deallocate (block_meta * toDeallocate);
void coalesceWithLeft (block_meta * rightBlock);
void coalesceWithRight(block_meta * leftBlock);

block_meta * findFirstFit(block_meta * curr, size_t size);
block_meta * findBestFit(block_meta * curr, size_t size);

void * ff_malloc (size_t size);
void * ff_free (void * toFree);

void * bf_malloc (size_t size);
void * bf_free (block_meta * toFree);


unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

