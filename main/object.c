//
// Created by aaron on 8/19/2024.
//
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

extern VM vm;

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    object->next = vm.objs;
    vm.objs = object;
    return object;
}
// Macro for allocating objects.
#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

/**
 * Initializes a new instance of a Lox Function
 * @return a pointer to said function object
 */
ObjFunction* newFunction() {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

/**
 * Creates a new function object from a prexisting
 * @param function
 * @return a pointer to the newly allocated object representing the function
 */
ObjNative* newNative(NativeFn function) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

/**
 * Allocates a Lox String, and interns it for future lookup
 * @param chars The raw C string we are allocating
 * @param length tne length of said string
 * @param hash The hash of said string
 * @return A pointer to the newly allocated Lox String
 */
static ObjString* allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    // String interning: Causes slight perf overhead for every allocation, but greatly improves performance when
    // Doing comoparisons (checking for function names)
    tableSet(&vm.strings, string, NIL_VAL);
    return string;
}

/**
 * Hashes a given string, using the FNV-1A hash function: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 * @param key The key we are hashing
 * @param length The number of characters to use when hashing
 * @return
 */
static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash ^= 16777619;
    }
}

/**
 * Copies a C string into an instance of a Lox String
 * @param chars The characters to copy from
 * @param length The amount of characters to read
 * @return A pointer to the aforementioned string
 */
ObjString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    // See if we have encountered this exact string before
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) return interned;
    // We have not, so we manually allocate enough space for the string and mark it as seen
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

static void printFunction(ObjFunction* function) {
    // handles the special case of the vm's own allocated function slot
    if (function->name == NULL) {
        printf("<script>");
        return;
    }
    printf("<fn> %s", function->name->chars);
}

/**
 * 
 * @param value 
 */
void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING: {
            printf("%s", AS_CSTRING(value));
            break;
        }
        case OBJ_FUNCTION: {
            printFunction(AS_FUNCTION(value));
            break;
        }
        case OBJ_NATIVE: {
            printf("<native fn>");
            break;
        }
        default: return;
    }
}

ObjString* takeString(char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocateString(chars, length, hash);
}