//
// Created by aaron on 8/6/2024.
//

#include "memory.h"
#include "vm.h"

#include <stdlib.h>

#include <memory.h>

#include "value.h"
#include "object.h"
#include "compiler.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
    }

    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);
    return result;
}

static void freeObject(Obj* object) {
#ifdef DEBUG_LOG_GC
    printf("%p freed, type %d\n", (void*)object, object->type);
#endif
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
            FREE(ObjFunction, object);
            break;
        }
        case OBJ_NATIVE: {
            FREE(ObjNative, object);
            break;
        }
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)object;
            FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalueCount);
            // Only frees the surrounding enclosure since multiple closures can contain the same exact function
            FREE(ObjClosure, object);
            break;
        }
        // Multiple closures could refer to the same value the upvalue refers to, so here we only free the wrapping struct
        case OBJ_UPVALUE: {
            FREE(ObjUpvalue, object);
            break;
        }
    }
}

void markObject(Obj* obj) {
    if (obj == NULL) return;
    obj->isMarked = true;

#ifdef DEBUG_LOG_GC
    printf("%p mark", (void*)obj);
    printValue(OBJ_VAL(obj));
    printf("\n");
#endif
}

void markValue(Value val) {
    if (IS_OBJ(val)) markObject(AS_OBJ(val));
}

static void markRoots() {
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        markValue(*slot);
    }
    markTable(&vm.globals);
    for (int i = 0; i < vm.frameCount; i++) {
        markObject((Obj*)vm.frames[i].closure);
    }
    for (ObjUpvalue* upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
        markObject((Obj*)upvalue);
    }
    markCompilerRoots();
}

void freeObjects() {
    Obj* object = vm.objs;
    while (object != NULL) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}

void collectGarbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin");
#endif

    // Marks the "roots" of the dyanmic memory
    markRoots();

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
#endif
}