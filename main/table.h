//
// Created by aaron on 8/27/2024.
//

#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
// In 'table', set 'key' to be associated with 'value'
bool tableSet(Table* table, ObjString* key, Value value);
void tableAddAll(Table* from, Table* to);
// Returns true if the key exists in the table, false otherwise. "value" is then set tp the value found
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableDelete(Table* table, ObjString* key);
// Checks to see if 'chars' already exists in our table. Mainly used for string interning.
ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);
// Helper function(s) for GC
// Note: Maybe move to memory header file instead?
void tableRemoveWhite(Table* table);
void markTable(Table* table);
#endif
