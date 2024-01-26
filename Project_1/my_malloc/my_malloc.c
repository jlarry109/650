#include "my_malloc.h"
// Global variables
FreeList freeList = { .head = NULL, .tail = NULL };
heap_info_t heap_info = { .totalAllocated = 0, .totalFreed = 0 };

bool isEmptyFreeList (FreeList * freeList) {
    return !freeList->head && !freeList->tail;
}

void initializeMemoryBlock(MemoryBlock * block, size_t dataSize, bool allocated) {
  block->dataSize = dataSize;
  block->allocated = allocated;
  block->prev = NULL;
  block->next = NULL;
}

void appendToFreeList(FreeList * list, MemoryBlock * toAdd) {
    //if list is empty, head and tail should both point to toAdd
  if (isEmptyFreeList(list)) {
    list->head = &(*toAdd);
    list->tail = &(*toAdd);
  } else {
    //Otherwise (if toAdd isn't first element in the list)
    list->tail->next = &(*toAdd);
    toAdd->prev = list->tail;
    toAdd->next = NULL;
    list->tail = &(*toAdd);
  }
  toAdd->allocated = false;
}

void insertIntoFreeList(FreeList* list, MemoryBlock* block, MemoryBlock* curr) {
  if (isEmptyFreeList(list)) {
    list->head = &(*block);
    list->tail = &(*block);
  } else if (curr == NULL) {
    list->head->prev = &(*block);
    block->next = list->head;
    block->prev = NULL;
    list->head = &(*block);
  } else if (curr == list->tail) {
    list->tail->next = &(*block);
    block->prev = list->tail;
    block->next = NULL;
    list->tail = &(*block);
  } else {
    MemoryBlock * toInsert = curr->next;
    curr->next = &(*block);
    block->prev = &(*curr);
    block->next = &(*toInsert);
    toInsert->prev = &(*block);
  }
}

void removeFromFreeList(FreeList* list, MemoryBlock* toRemove) {
    if (isEmptyFreeList(list)) { 
        fprintf(stderr, "Can't remove from an empty list\n"); 
        return; 
    }
  if (list->head == toRemove && list->tail == toRemove) {//else if toRemove is the first element in the list
    list->head = NULL;
    list->tail = NULL;
  } else if (toRemove == list->head) {//else if toRemove is the first element in the list
    list->head = list->head->next;
    list->head->prev = NULL;
  } else if (toRemove == list->tail) {
    list->tail = list->tail->prev;
    list->tail->next = NULL;
  } else {//toRemove is somewhere in between first and last items in list
    toRemove->prev->next = toRemove->next;
    toRemove->next->prev = toRemove->prev;
  }

  toRemove->prev = NULL;
  toRemove->next = NULL;
  toRemove->allocated = true;
}


void* allocateMemory(size_t dataSize) {
  size_t totalSize = dataSize + META_SIZE;
  MemoryBlock* allocated = sbrk(totalSize);

  if (allocated == (void*)(-1) || errno == ENOMEM) {
    fprintf(stderr, "sbrk failed to allocate memory\n");
    return NULL;
  }

  initializeMemoryBlock(allocated, dataSize, true);
  heap_info.totalAllocated += totalSize;
  return allocated + 1;  //Return the pointer to the start of the actual data not the metadata. This pointer arithmetic is essentially equal to (char *)allocatedBlock + META_SIZE
}

MemoryBlock* splitMemoryBlock(MemoryBlock* block, size_t dataSize) {
  if (dataSize + META_SIZE > block->dataSize) {
    heap_info.totalFreed -= (META_SIZE + block->dataSize);
    removeFromFreeList(&freeList, block);
  } else {
    MemoryBlock* remainingBlock = (MemoryBlock*)((char*)(block + 1) + dataSize);
    size_t remainingSize = block->dataSize - dataSize - META_SIZE;
    initializeMemoryBlock(remainingBlock, remainingSize, false);
    block->dataSize = dataSize;
    block->allocated = true;
    insertIntoFreeList(&freeList, remainingBlock, block);
    removeFromFreeList(&freeList, block);

    heap_info.totalFreed -= (META_SIZE + dataSize);
  }
  return block;
}

void coalesceWithLeft(MemoryBlock* block) {
  if (block->prev && (char*)block == (char*)block->prev + META_SIZE + block->prev->dataSize) {
    MemoryBlock* leftBlock = block->prev;
    leftBlock->dataSize += META_SIZE + block->dataSize;
    removeFromFreeList(&freeList, block);
  }
}

void coalesceWithRight(MemoryBlock* block) {
  if (block->next && (char*)block->next == (char*)block + META_SIZE + block->dataSize) {
    MemoryBlock* rightBlock = block->next;
    block->dataSize += META_SIZE + rightBlock->dataSize;
    removeFromFreeList(&freeList, rightBlock);
  }
}

void freeMemoryBlock(MemoryBlock* block) {
  block->allocated = false;
  heap_info.totalFreed += block->dataSize + META_SIZE;
  if (isEmptyFreeList(&freeList) || block > freeList.tail) {
    appendToFreeList(&freeList, block);
    bool coalesce = (!isEmptyFreeList(&freeList) && block > freeList.tail)? true : false;
    if (coalesce) {
      coalesceWithLeft(block);
    }
  } else if (block < freeList.head) {
    insertIntoFreeList(&freeList, block, NULL);
    coalesceWithRight(block);
  }
   else {
      MemoryBlock* iter = freeList.head;
      while (iter < block) {
        iter = iter->next;
      }
      MemoryBlock* curr = iter->prev;
      insertIntoFreeList(&freeList, block, curr);
      coalesceWithRight(block);
      coalesceWithLeft(block);
  }
}

void* ff_malloc(size_t size) {
    if (size == 0) { return NULL; }
  MemoryBlock* curr = freeList.head;
  curr = findFirstFit(curr, size);
    if (curr != NULL) {
        return splitMemoryBlock(curr, size) + 1;
    }
  return allocateMemory(size);
}

void ff_free (void* ptr) {
  if (ptr == NULL) {
    return;
  }
  MemoryBlock* block = (MemoryBlock*)((char*)ptr - META_SIZE);
  if (block->allocated == true) {
    freeMemoryBlock(block);
  }
}
MemoryBlock * findFirstFit(MemoryBlock * curr, size_t size) {
    while (curr != NULL) {
        if (curr->dataSize >= size) {
            return curr;
        }
        curr = curr->next;
    }
    return curr;
}

MemoryBlock * findBestFit(MemoryBlock * curr, size_t size){
    MemoryBlock * bestFit = NULL;
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

void* bf_malloc(size_t size) {
    if (size == 0) { return NULL; }
    MemoryBlock* current = freeList.head;
    MemoryBlock * bestFit = findBestFit(current, size);
    if (bestFit != NULL) {
        return splitMemoryBlock(current, size) + 1;
      }
    return allocateMemory(size);
}

void bf_free(void* ptr) {
  ff_free(ptr);
}

unsigned long get_data_segment_size() {
  return heap_info.totalAllocated;
}

unsigned long get_data_segment_free_space_size() {
  return heap_info.totalFreed;
}
