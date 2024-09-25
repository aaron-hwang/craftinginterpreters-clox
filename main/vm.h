//
// Created by aaron on 8/10/2024.
//

/**
 * Header file for the functions and structs that represent our virtual machine.
 */

#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "object.h"
#include "table.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
	ObjClosure* closure;
	// Current instruction pointer relative to the start of this callstack/frame
	uint8_t* ip;
	Value* slots;
} CallFrame;

typedef struct {
	CallFrame frames[FRAMES_MAX];
	int frameCount;

    Value stack[STACK_MAX];
    Value* stackTop;
    Obj* objs;
    // Keeps track of all strings recorded so far, for string interning
    Table strings;
    // Keeps track of all global variables
    Table globals;
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
