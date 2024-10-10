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

// Technically arbitrary, for performance ideally profile and test different factors
#define GC_HEAP_GROW_FACTOR 2

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    vm.bytesAllocated += newSize - oldSize;
    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
    }

    if (vm.bytesAllocated >= vm.nextGC) {
        collectGarbage();
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
        case OBJ_CLASS: {
            // The name itself might still be in use
            FREE(ObjClass, object);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)object;
            freeTable(&instance->fields);
            FREE(ObjInstance, object);
            break;
        }
    }
}

void markObject(Obj* obj) {
    // Second condition avoids cycles
    if (obj == NULL || obj->isMarked) return;
    obj->isMarked = true;

    if (vm.grayCapacity < vm.grayCount + 1) {
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        // Manage the stack ourselves to avoid recursive gc
        vm.grayStack = (Obj**)realloc(vm.grayStack, sizeof(Obj*) * vm.grayCapacity);
        // Basically screwed if this fails
        if (vm.grayStack == NULL) exit(1);
    }

    vm.grayStack[vm.grayCount++] = obj;

#ifdef DEBUG_LOG_GC
    printf("%p mark", (void*)obj);
    printValue(OBJ_VAL(obj));
    printf("\n");
#endif
}

void markValue(Value val) {
    if (IS_OBJ(val)) markObject(AS_OBJ(val));
}

static void markArray(ValueArray* array) {
    for (int i = 0; i < array->count; i++) {
        markValue(array->values[i]);
    }
}

static void markRoots() {
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        markValue(*slot);
    }
    // We intentionally do not mark our table of interned strings, since they are a little special.
    // Marking them normally would lead to us never collecting any strings
    // manually marking is also bad since we would just have a bunch of dangling pointers
    markTable(&vm.globals);
    for (int i = 0; i < vm.frameCount; i++) {
        markObject((Obj*)vm.frames[i].closure);
    }
    for (ObjUpvalue* upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
        markObject((Obj*)upvalue);
    }
    markCompilerRoots();
}

// A black object is any object whose 'isMarked' field is set, and is no longer in the gray stack of the vm
static void blackenObject(Obj* obj) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)obj);
    printValue(OBJ_VAL(obj));
    printf("\n");
#endif


    switch (obj->type) {
        case OBJ_UPVALUE: {
            markValue(((ObjUpvalue*)obj)->closed);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)obj;
            markObject((Obj*)function->name);
            markArray(&function->chunk.constants);
            break;
        }
        case OBJ_CLOSURE: {
            ObjClosure* closure = (ObjClosure*)obj;
            markObject((Obj*)closure->function);
            for (int i = 0; i < closure->upvalueCount; i++) {
                markObject((Obj*)closure->upvalues[i]);

            }
            break;
        }
        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*)obj;
            markObject((Obj*)klass->name);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* instance = (ObjInstance*)obj;
            markObject((Obj*)instance->klass);
            markTable(&instance->fields);
            break;
        }
        // Nothing to do
        case OBJ_NATIVE:
        case OBJ_STRING:
            break;
    }
}

static void traceReferences() {
    while (vm.grayCount > 0) {
        Obj* object = vm.grayStack[--vm.grayCount];
        blackenObject(object);
    }
}

void freeObjects() {
    Obj* object = vm.objs;
    while (object != NULL) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }

    free(vm.grayStack);
}

static void sweep() {
    Obj* previous = NULL;
    Obj* obj = vm.objs;
    while (obj != NULL) {
        if (obj->isMarked) {
            obj->isMarked = false;
            previous = obj;
            obj = obj->next;
        } else {
            Obj* unreached = obj;
            obj = obj->next;
            if (previous != NULL) {
                previous->next = obj;
            } else {
                vm.objs = obj;
            }

            freeObject(unreached);
        }
    }
}

// The main garbage collection funtion
/**
 * High level overview of how it works:
 * Using the tricolor abstraction, every object we dynamically allocate memory for can be in one of three
 * states:
 * White - The object has not yet been traversed or encountered by our GC algorithm
 * Gray - The object is reachable (should not be collected), but we have not explored this node's neighbors
 * Black - After an object is marked and all of its references are marked
 *
 * In other words,
 * 1. Start with every node white
 * 2. Visit all roots, marking them grey,
 * 3. Visit all gray nodes, visit their references
 * 4. Mark the original grey node black
 * Repeat 3 and 4 while grey nodes exist
 * 5. Any white objects remaining can be gc'ed
 * It can be seen that a black object will never point to a white object according to the above rules:
 * tricolor invariant
 *
 * As memory is allocated and freed during the program's lifetime, the frequency with which GC is run automatically
 * adjusts, increasing with less memory and decreasing with more.
 */
void collectGarbage() {
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t prev = vm.bytesAllocated;
#endif

    // Marks the "roots" of the dyanmic memory as grey
    markRoots();
    // Steps 3 and 4
    traceReferences();
    tableRemoveWhite(&vm.strings);
    // step 5
    sweep();

    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("    Collected %zu bytes (from %zu to %zu) next at %zu\n",
        prev-vm.bytesAllocated, prev, vm.bytesAllocated, vm.nextGC);
#endif
}