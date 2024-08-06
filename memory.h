//
// Created by aaron on 8/6/2024.
//

#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

#define GROW_CAPACITY(capacity) \
((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * oldCount, \
    sizeof(type) * newCount)

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * oldCount, 0)

/**
 * Reallocate a new chunk of size newSize
 * @param pointer A pointer to the old chunk
 * @param oldSize The previous size of the chunk
 * @param newSize The new size of the chunk
 * @return pointer of the same type as pointer
 *
 * There are four different cases to be aware of:
 * oldSize == 0 && newSize !=0 should allocate a new block
 * oldSize != 0 && newSize ==0 should free the block
 * oldSize != 0 && newSize < oldSize should shrink the existing allocation
 * oldSize != 0 && newSize > oldSize should grow the existing allocation
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif
