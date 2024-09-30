//
// Created by aaron on 8/19/2024.
//
/**
 * This file holds all the structs and useful macros for defining what constitutes as an 'Object' in Lox.
 * This is where we also hold most/all of the runtime representations for core language features, like functions, closures, and upvalues.
 */
#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) \
    ((ObjNative*)AS_OBJ(value))->function
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    // Special type of object, native functions
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;
    bool isMarked;
};

// In lox, functions are first class.
typedef struct {
    Obj obj;
    // The number of parameters a function expects
    int arity;
    int upvalueCount;
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

// The runtime representation of an upvalue
typedef struct ObjUpvalue {
    Obj obj;
    // Important that this is a pointer, as it needs to be aware of any changes to the variable that happen at runtime
    Value* location;
    Value closed;
    // Each open upvalue points to the next open upvalue referencing a local var farther in the stack
    struct ObjUpvalue* next;
} ObjUpvalue;

ObjUpvalue* newUpvalue(Value* slot);

// A struct that represents the closure for a given function
typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    // More for the GC than anything else, because technically the function already knows their own upvalue count
    int upvalueCount;
} ObjClosure;

ObjClosure* newClosure(ObjFunction* function);

ObjString* copyString(const char* chars, int length);
ObjString* takeString(char* chars, int length);
void printObject(Value value);

// Cannot directly put this in the macro, as it will evaluate whatever "value" is multiple times.
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}



#endif
