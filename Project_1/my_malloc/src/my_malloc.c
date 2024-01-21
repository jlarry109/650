#include "my_malloc.h"

heap_info_t heap_info = { .totalAllocated = 0, .totalFreed = 0 };
LinkedList free_list = { .head = NULL, .tail = NULL};

void displayBlockMeta(block_meta * block) {
    printf("curr_blk: %p, allocated: %d, data_size: %zu, prev_blk: %p, next_blk: %p\n", block, block->allocated, block->dataSize, block->prev, block->next);
}

void initBlockMeta(block_meta * block, size_t dataSize, bool allocated) {
    block->dataSize = dataSize;
    block->allocated = allocated;
    block->prev = NULL;
    block->next = NULL;
}

bool isEmpty (LinkedList * list) {
    return !list->head && !list->tail;
}
void appendToFreeList(LinkedList * list, block_meta * toAppend){
    //if list is empty, head and tail should both point to toAdd
    if (isEmpty(list)) {
        list->head = &(*toAppend);
        list->tail = &(*toAppend);
    }
    //Otherwise (if toAdd isn't first element in the list)
    list->tail->next = &(*toAppend);
    toAppend->next = NULL;
    toAppend->prev = list->tail;
    list->tail = &(*toAppend);
}

void removeFromFreeList(LinkedList * list, block_meta * toRemove) {
    if (isEmpty(list)) {
        fprintf(stderr, "Can't remove from an empty list\n"); 
        return; 
    }
    if(list->head = toRemove && list->tail == toRemove) { //if toRemove is the only item in list
        list->head = NULL;
        list -> tail = NULL;
    }
    else if (list->head = toRemove) {//else if toRemove is the first element in the list
        list->head = list->head->next;
        list->head->prev = NULL;
    }
    else if (list->tail = toRemove) { //
        list->tail = list->tail->prev;
        list->tail->next = NULL;
    }
    else {//toRemove is somewhere in between first and last items in list
        toRemove->prev->next = toRemove->next;
        toRemove->next->prev = toRemove->prev;
    }
    toRemove->prev = NULL;
    toRemove->next = NULL;
    toRemove->allocated = true;
}

void insertInFrontOf(LinkedList * list, block_meta * toInsert, block_meta * curr) {
    if ((!list->head && !list->tail) || (curr && list->tail == curr)) {
        AddToFreeList(list, toInsert);
    }
    else if (curr == NULL) {
        list->head->prev = toInsert;
        toInsert = list->head;
        toInsert->prev = NULL;
        list->head = toInsert;
    }
    else {
        block_meta * temp = curr->next;
        curr->next = toInsert;
        toInsert->prev = curr;
        toInsert->next = temp;
        temp->prev = toInsert;
    }
}
void printFreeList(LinkedList * toPrint) {
    printf("head: %p\n", toPrint->head);
    block_meta * iter = toPrint->head;
    while (iter) {
        displayBlockMeta(iter);
        iter = iter->next;
    }
    printf("tail: %p\n", toPrint->head);
}


void * allocate (size_t dataSize) {
    //A blockSize comprises of the size of its data and the size of its metadata
    size_t blockSize = dataSize + META_SIZE;
    block_meta * allocatedBlock = sbrk(blockSize);
    if (allocatedBlock == (void *) -1 || errno == ENOMEM) {
        fprintf(stderr, "sbrk failed to allocate memory\n");
        return NULL;
    }
    //Initialize block metadata
    initBlockMeta(allocatedBlock, dataSize, true);
    heap_info.totalAllocated += blockSize;
    return allocatedBlock + 1; //Return the pointer to the start of the actual data not the metadata. This pointer arithmetic is essentially equal to (void *)allocatedBlock + META_SIZE
}

void * deallocate (block_meta * toDeallocate) {
    //if free list is empty, then append to free list
    if (isEmpty(&free_list) || (toDeallocate > free_list.tail)) {
        bool coalesce = (!isEmpty(&free_list) && toDeallocate > free_list.tail)? true : false;
        appendToFreeList(&free_list, toDeallocate);
        if (coalesce) {
            coalesceWithLeft(toDeallocate);
        }
    }
    else if (toDeallocate < free_list.head) {
        insertInFrontOf(&free_list, toDeallocate, NULL);
        coalesceWithRight(toDeallocate);
    }
    else {
        block_meta * iter = free_list.head;
        while (iter < toDeallocate) {
            iter = iter->next;
        }
        //get a pointer to the biggest address/ ptr smaller than the address of the address to deallocated
        block_meta * curr = iter->prev;
        insertInFrontOf(&free_list, toDeallocate, curr);
        coalesceWithRight(toDeallocate);
        coalesceWithLeft(toDeallocate);
    }
    toDeallocate->allocated = false;
    heap_info.totalFreed -= toDeallocate->dataSize + META_SIZE;
}

block_meta * findFirstFit(block_meta * curr, size_t size) {
    while (curr != NULL) {
        if (curr->dataSize >= size) {
            return curr;
        }
        curr = curr->next;
    }
    return curr;
}

block_meta * findBestFit(block_meta * curr, size_t size){
    block_meta * bestFit = NULL;
    while (curr != NULL) {
        if (curr->dataSize == size) {
            bestFit = curr;
            break;
        }
        if (curr->dataSize > size) {
            if (bestFit == NULL) {
                bestFit = curr;
            } else if (bestFit->dataSize > curr->dataSize) {
                bestFit = curr;
            }
        }
    curr = curr->next;
  }
  return bestFit;
}

void * splitBlock(block_meta * block, size_t size) {
    if (size == block->dataSize) {//then no need to split just return the block
        removeFromFreeList(&free_list, block);
        heap_info.totalFreed -= block->dataSize + META_SIZE;
    }
    else {
        //remainFree starts at (size + META_SIZE) away from the start of the current block
        block_meta * remainFree = (block_meta *)((char *)(block) + size + META_SIZE); 
        size_t remainFreeSize = block->dataSize - size;//size of remainFree block 
        initBlockMeta(remainFree, remainFreeSize, false);//initialize remainFree block
        insertInFrontOf(&free_list, remainFree, block); //add remainFree to list of free blocks
        removeFromFreeList(&free_list, block); //remove allocated block from list of free blocks
        heap_info.totalFreed -= size + META_SIZE;

        block->dataSize = size;
        block->allocated = true;

    }
    return block;
}
void coalesceWithLeft (block_meta * rightBlock) {
    if (rightBlock->prev && (char *)rightBlock == (char *)rightBlock->prev + rightBlock->prev->dataSize + META_SIZE) {
        rightBlock->prev->dataSize += rightBlock->dataSize + META_SIZE;
        removeFromFreeList(&free_list, rightBlock);
    } 
}
void coalesceWithRight(block_meta * leftBlock){
        if (leftBlock->next && (char *)leftBlock == (char *)leftBlock->next - leftBlock->dataSize - META_SIZE) {
            block_meta * rightBlock = leftBlock->next;
            leftBlock->dataSize += rightBlock->dataSize + META_SIZE;
            removeFromFreeList(&free_list, rightBlock);
        }
}

void * ff_malloc (size_t size) {
    block_meta * curr = free_list.head;
    curr = findFirstFit(curr, size);
    if (curr != NULL) {
        return splitBlock(curr, size) + 1;
    }
    return allocate(size);
}
void * ff_free (void * toFree){
    if (toFree == NULL) {
        return;
    }
}

void * bf_malloc (size_t size) {
    block_meta * curr = free_list.head;
    block_meta * bestFit = findBestFit(curr, size);
    if (bestFit != NULL) {
        return splitBlock(curr, size) + 1;
    }
    return allocate(size);
}
void * bf_free (block_meta * toFree);


unsigned long get_data_segment_size() {
    return heap_info.totalAllocated;
}
unsigned long get_data_segment_free_space_size() {
    return heap_info.totalFreed;
}

