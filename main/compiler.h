//
// Created by aaron on 8/15/2024.
//

#ifndef clox_compiler_h
#define clox_compiler_h
#include "chunk.h"
#include "object.h"

ObjFunction* compile(const char* source);
// Helper function for GC
void markCompilerRoots();

#endif //COMPILER_H
