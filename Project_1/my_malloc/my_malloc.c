#include "my_malloc.h"
// Global variables
FreeList freeList = { .head = NULL, .tail = NULL };
heap_info_t heap_info = { .totalAllocated = 0, .totalFreed = 0 };

bool isEmptyFreeList (FreeList * freeList) {
    return !freeList->head && !freeList->tail;
}
// Memory block methods
void initializeMemoryBlock(MemoryBlock * block, size_t dataSize, bool allocated) {
  block->dataSize = dataSize;
  block->allocated = allocated;
  block->prev = NULL;
  block->next = NULL;
}
void displayMemoryBlock(MemoryBlock* block) {
  printf("Block: %p, Size: %lu, Prev: %p, Next: %p, Occupied: %d\n",
         block, block->dataSize, block->prev, block->next, block->allocated);
}

// FreeList methods
void displayFreeList(FreeList* list) {
  printf("Free List:\n");
  MemoryBlock* current = list->head;
  while (current != NULL) {
    displayMemoryBlock(current);
    current = current->next;
  }
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
    list->head = block;
  } else if (curr == list->tail) {
    list->tail->next = &(*block);
    block->prev = list->tail;
    block->next = NULL;
    list->tail = &(*block);
  } else {
    MemoryBlock * toInsert = curr->next;
    curr->next = &(*block);
    block->prev = curr;
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
  return allocated + 1;  //Return the pointer to the start of the actual data not the metadata. This pointer arithmetic is essentially equal to (void *)allocatedBlock + META_SIZE
}

MemoryBlock* splitMemoryBlock(MemoryBlock* block, size_t dataSize) {
  if (dataSize + META_SIZE > block->dataSize) {
    heap_info.totalFreed -= (META_SIZE + block->dataSize);
    removeFromFreeList(&freeList, block);
  } else {
    MemoryBlock* remainingBlock = (MemoryBlock*)((char*)block + META_SIZE + dataSize);
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
    MemoryBlock* prevBlock = block->prev;
    prevBlock->dataSize += META_SIZE + block->dataSize;
    removeFromFreeList(&freeList, block);
  }
}

void coalesceWithRight(MemoryBlock* block) {
  if (block->next && (char*)block->next == (char*)block + META_SIZE + block->dataSize) {
    MemoryBlock* nextBlock = block->next;
    block->dataSize += META_SIZE + nextBlock->dataSize;
    removeFromFreeList(&freeList, nextBlock);
  }
}

void freeMemoryBlock(MemoryBlock* block) {
  heap_info.totalFreed += block->dataSize + META_SIZE;

  block->allocated = false;

  if (isEmptyFreeList(&freeList)) {
    appendToFreeList(&freeList, block);
  } else if (block < freeList.head) {
    insertIntoFreeList(&freeList, block, NULL);
    coalesceWithRight(block);
  } else if (block > freeList.tail) {
    appendToFreeList(&freeList, block);
    coalesceWithLeft(block);
  } else {
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
  MemoryBlock* current = freeList.head;
  while (current != NULL) {
    if (current->allocated == false && current->dataSize >= size) {
      return (char*)splitMemoryBlock(current, size) + META_SIZE;
    }
    current = current->next;
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

void* bf_malloc(size_t size) {
    if (size == 0) { return NULL; }
  MemoryBlock* current = freeList.head;
  MemoryBlock* bestFit = NULL;
  size_t minSize = SIZE_MAX;

  while (current != NULL) {
    if (current->allocated == false && current->dataSize >= size) {
      if (current->dataSize == size) {
        return splitMemoryBlock(current, size) + 1;
        //return (char*)splitMemoryBlock(current, size) + META_SIZE;
      }
      if (current->dataSize < minSize) {
        bestFit = current;
        minSize = current->dataSize;
      }
    }
    current = current->next;
  }
  if (bestFit == NULL) {
    return allocateMemory(size);
  } else {
    return (char*)splitMemoryBlock(bestFit, size) + META_SIZE;
  }
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
