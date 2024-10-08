//
// Created by aaron on 8/6/2024.
//

#include <stdlib.h>
#include "chunk.h"
#include "memory.h"
#include "vm.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        //naive and inefficient method of keeping track of lines that generated this bytecode, TODO change implementation
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    freeValueArray(&chunk->constants);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    initChunk(chunk);
}

// Returns the index where the given value was appended
int addConstant(Chunk* chunk, Value value) {
    // Each chunk owns a constant table, which is a dynamic array. That might need to grow and call
    // reallocate, which can trigger a gc. That gc may sweep up "value" even though we still need it.
    // Popping it onto the stack before we try to reallocate fixes this.
    push(value);
    writeValueArray(&chunk->constants, value);
    pop(value);
    return chunk->constants.count - 1;
}

