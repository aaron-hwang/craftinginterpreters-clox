//
// Created by aaron on 8/19/2024.
//

#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) \
    ((ObjNative*)AS_OBJ(value))->function

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    // Special type of object, native functions
    OBJ_NATIVE,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;
};

// In lox, functions are first class.
typedef struct {
    Obj obj;
    // The number of parameters a function expects
    int arity;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

// We need these for native functions. Basically wrappers for native C code.
typedef Value (*NativeFn)(int argc, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

ObjNative* newNative(NativeFn native);

ObjFunction* newFunction();

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

ObjString* copyString(const char* chars, int length);
ObjString* takeString(char* chars, int length);
void printObject(Value value);

// Cannot directly put this in the macro, as it will evaluate whatever "value" is multiple times.
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}



#endif
