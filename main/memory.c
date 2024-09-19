//
// Created by aaron on 8/6/2024.
//

#include "memory.h"
#include "vm.h"

#include <stdlib.h>

#include <memory.h>

#include "value.h"
#include "object.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);
    return result;
}

static void freeObject(Obj* object) {
    switch (object->type) {
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            FREE_ARRAY(char, string->chars, string->length+1);
            FREE(ObjString, object);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*) object;
            freeChunk(&function->chunk);
            FREE(OBJ_FUNCTION, object);
            break;
        }
    }
}

void freeObjects() {
    Obj* object = vm.objs;
    while (object != NULL) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}