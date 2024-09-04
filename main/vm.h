//
// Created by aaron on 8/10/2024.
//

/**
 * Header file for the functions and structs that represent our virtual machine.
 */

#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    // Instruction pointer
    uint8_t* ip;

    Value stack[STACK_MAX];
    Value* stackTop;
    Obj* objs;
    // Keeps track of all strings recorded so far, for string interning
    Table strings;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif
