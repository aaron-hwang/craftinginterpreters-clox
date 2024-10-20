//
// Created by aaron on 8/6/2024.
//

#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

// potentially todo: implement seperate opcodes for "<=", ">=", "!=" for better perf
typedef enum {
    OP_RETURN,
    OP_CLASS,
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_SET_PROPERTY,
    OP_GET_PROPERTY,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_NOT,
    OP_METHOD,
    // A fusion of OP_GET_PROPERTY and OP_CALL (Invoked when you do className.method(args))
    OP_INVOKE,
} OpCode;

typedef struct {
    int count;
    int capacity;
    uint8_t *code;
    int* lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

#endif