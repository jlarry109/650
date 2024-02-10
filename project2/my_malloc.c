#include "my_malloc.h"
//Global variables
MemoryBlock * head = NULL;
MemoryBlock * tail = NULL;
__thread MemoryBlock * ts_nolockHead = NULL;
__thread MemoryBlock * ts_nolockTail = NULL;
FreeList freeList = { .head = NULL, .tail = NULL };


bool isEmptyFreeList (MemoryBlock * head, MemoryBlock * tail) {
    return !head && !tail;
}

void initializeMemoryBlock(MemoryBlock * block, size_t dataSize, bool allocated) {
  block->dataSize = dataSize;
  block->allocated = allocated;
  block->prev = NULL;
  block->next = NULL;
}

void appendToFreeList(MemoryBlock * head, MemoryBlock * tail, MemoryBlock * toAdd) {
    //if list is empty, head and tail should both point to toAdd
  if (isEmptyFreeList(head, tail)) {
      head = &(*toAdd);
      tail = &(*toAdd);
  } else {
    //Otherwise (if toAdd isn't first element in the list)
      tail->next = &(*toAdd);
      toAdd->prev = tail;
      toAdd->next = NULL;
      tail = &(*toAdd);
  }
  toAdd->allocated = false;
}

void insertIntoFreeList(MemoryBlock * head, MemoryBlock * tail, MemoryBlock * toInsert, MemoryBlock* curr) {
  if (isEmptyFreeList(head, tail)) {
      head = &(*toInsert);
      tail = &(*toInsert);
  } else if (curr == NULL) {
      head->prev = &(*toInsert);
      toInsert->next = head;
      toInsert->prev = NULL;
      head = &(*toInsert);
  } else if (curr == tail) {
    tail->next = &(*toInsert);
    toInsert->prev = tail;
    toInsert->next = NULL;
    tail = &(*toInsert);
  } else {
    MemoryBlock * toInsert = curr->next;
    curr->next = &(*toInsert);
    toInsert->prev = &(*curr);
    toInsert->next = &(*toInsert);
    toInsert->prev = &(*toInsert);
  }
}

void removeFromFreeList(MemoryBlock * head, MemoryBlock * tail, MemoryBlock* toRemove) {
    if (isEmptyFreeList(head, tail)) { 
        fprintf(stderr, "Can't remove from an empty list\n"); 
        return; 
    }
  if (head == toRemove && tail == toRemove) {//else if toRemove is the first element in the list
      head = NULL;
      tail = NULL;
  } else if (toRemove == head) {//else if toRemove is the first element in the list
    head = head->next;
    head->prev = NULL;
  } else if (toRemove == tail) {
    tail = tail->prev;
    tail->next = NULL;
  } else {//toRemove is somewhere in between first and last items in list
    toRemove->prev->next = toRemove->next;
    toRemove->next->prev = toRemove->prev;
  }
  toRemove->prev = NULL;
  toRemove->next = NULL;
  toRemove->allocated = true;
}


void* allocateMemory(size_t dataSize, sbrkFuncPtr funcPtr) {
  size_t totalSize = dataSize + META_SIZE;
  MemoryBlock* allocated = (*funcPtr)(totalSize);

  if (allocated == (void*)(-1) || errno == ENOMEM) {
    fprintf(stderr, "sbrk failed to allocate memory\n");
    return NULL;
  }

  initializeMemoryBlock(allocated, dataSize, true);
  return allocated + 1;  //Return the pointer to the start of the actual data not the metadata. This pointer arithmetic is essentially equal to (char *)allocatedBlock + META_SIZE
}

MemoryBlock* splitMemoryBlock(MemoryBlock * head, MemoryBlock * tail, MemoryBlock* block, size_t dataSize) {
  if (block->dataSize < META_SIZE + dataSize) {
      removeFromFreeList(head, tail, block);
  } else {
      MemoryBlock * remainingBlock = (MemoryBlock *)((char*)(block + 1) + dataSize);
      size_t remainingSize = block->dataSize - dataSize - META_SIZE;
      initializeMemoryBlock(remainingBlock, remainingSize, false);
      block->dataSize = dataSize;
      block->allocated = true;
      insertIntoFreeList(head, tail, remainingBlock, block);
      removeFromFreeList(head, tail, block);
  }
  return block;
}

void coalesceWithLeft(MemoryBlock * head, MemoryBlock * tail, MemoryBlock* block) {
  if (block->prev && (char*)block == (char*)block->prev + META_SIZE + block->prev->dataSize) {
    MemoryBlock * leftBlock = block->prev;
    leftBlock->dataSize += META_SIZE + block->dataSize;
    removeFromFreeList(head, tail, block);
  }
}

void coalesceWithRight(MemoryBlock * head, MemoryBlock * tail, MemoryBlock* block) {
  if (block->next && (char*)block->next == (char*)block + META_SIZE + block->dataSize) {
    MemoryBlock * rightBlock = block->next;
    block->dataSize += META_SIZE + rightBlock->dataSize;
    removeFromFreeList(head, tail, rightBlock);
  }
}

void freeMemoryBlock(MemoryBlock * head, MemoryBlock * tail, MemoryBlock * block) {
  block->allocated = false;
  if (isEmptyFreeList(head, tail) || block > tail) {
    appendToFreeList(head, tail , block);
    bool coalesce = (!isEmptyFreeList(head, tail) && block > tail)? true : false;
    if (coalesce) {
      coalesceWithLeft(head, tail, block);
    }
  } else if (block < head) {
    insertIntoFreeList(head, tail , block, NULL);
    coalesceWithRight(head, tail, block);
  }
   else {
      MemoryBlock * iter = head;
      while (iter < block) {
        iter = iter->next;
      }
      MemoryBlock * curr = iter->prev;
      insertIntoFreeList(head, tail, block, curr);
      coalesceWithRight(head, tail, block);
      coalesceWithLeft(head, tail, block);
  }
}

void * ff_malloc(MemoryBlock * head, MemoryBlock * tail, sbrkFuncPtr funcPtr, size_t size) {
    if (size == 0) { return NULL; }
    MemoryBlock * curr = head;
    curr = findFirstFit(head, tail, curr, size);
    if (curr != NULL) {
      // thread-safe?
        return splitMemoryBlock(head, tail, curr, size) + 1;
    }
          //Thread-safe?
    return allocateMemory(size, (*funcPtr));
}

void ff_free (MemoryBlock * head, MemoryBlock * tail, void * ptr) {
    if (ptr == NULL) {
      return;
    }
    MemoryBlock * block = (MemoryBlock *)(ptr) - 1;
    //MemoryBlock * block = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
    if (block->allocated == true) {
      freeMemoryBlock(head, tail, block);
    }
}
MemoryBlock * findFirstFit(MemoryBlock * head, MemoryBlock * tail, MemoryBlock * curr, size_t size) {
    while (curr != NULL) {
        if (curr->dataSize >= size) {
            return curr;
        }
        curr = curr->next;
    }
    return curr;
}

MemoryBlock * findBestFit(MemoryBlock * head, MemoryBlock * tail, MemoryBlock * curr, size_t size){
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

void * bf_malloc(MemoryBlock * head, MemoryBlock * tail, sbrkFuncPtr funcPtr, size_t size) {
    if (size == 0) { return NULL; }
    MemoryBlock * current = head;
    MemoryBlock * bestFit = findBestFit(head, tail, current, size);
    if (bestFit != NULL) {
        return splitMemoryBlock(head, tail, bestFit, size) + 1;
      }
    return allocateMemory(size, (*funcPtr));
}


void * sbrk_nolock (size_t size) {
  assert(pthread_mutex_lock(&lock) == 0);
  void * ptr = sbrk(size);
  if (ptr == (void*)(-1) || errno == ENOMEM) {
    fprintf(stderr, "sbrk failed to allocate memory\n");
    return NULL;
  } 
  assert(pthread_mutex_unlock(&lock) == 0);
  return ptr;
}
void bf_free(MemoryBlock * head, MemoryBlock * tail, void * ptr) {
  ff_free(head, tail, ptr);
}

/* Thread-Safe Malloc and Free */
void * ts_malloc_lock (size_t size) {
  if (pthread_mutex_lock(&lock) != 0) {
    fprintf(stderr, "Failed to lock cs \n");
     return NULL;
  }
  void * allocatedBlock =  bf_malloc(head, tail, sbrk, size);
  if (pthread_mutex_unlock(&lock) != 0) {
      fprintf(stderr, "Failed to unlock cs \n");
      return NULL;
  }
  return allocatedBlock;
}
void bf_freee(MemoryBlock * head, MemoryBlock * tail, MemoryBlock * ptr) {

}
void ts_free_lock(void * ptr) {
  if (pthread_mutex_lock(&lock) != 0) {
      fprintf(stderr, "Failed to lock cs \n");
  }
  bf_free(head, tail, ptr);
  if (pthread_mutex_unlock(&lock) != 0) {
      fprintf(stderr, "Failed to unlock cs \n");
      exit(EXIT_FAILURE);
  } 
}  

void * ts_malloc_nolock (size_t size) {
    void * allocated = bf_malloc(ts_nolockHead, ts_nolockTail, sbrk_nolock, size);
    return allocated;
}

void * ts_free_nolock(void * ptr) {
    bf_freee(ts_nolockHead, ts_nolockTail, ptr);
}
