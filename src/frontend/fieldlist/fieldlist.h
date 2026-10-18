#ifndef FIELDLIST_H
#define FIELDLIST_H
#include <typetable.h>
#include <stdio.h>
#include <stdlib.h>
#include <classtable.h>

typedef struct FieldList {
    char *name;
    int fieldIndex;
    int size;
    struct TypeTable *type;
    struct FieldList *next;
    struct ClassTable *ctype; // for classes (if the member itself is a variable)
} FieldList;

extern struct FieldList* FieldListHead;
extern struct FieldList* FieldListTail;
extern int fieldIndexCounter;

/**
 * Appends a new field to the list.
 * @return the new entry, or NULL if allocation fails
 */
struct FieldList* FieldListAppend(char* name, struct TypeTable* type);
struct FieldList* FieldListAppendClass(char* name, struct ClassTable* ctype);
/**
 * @return the head of the FieldList
 */
struct FieldList* FieldListReturnHead();

void FieldListReset();

void FieldListDestroy();
#endif
