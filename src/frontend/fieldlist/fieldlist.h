#ifndef FIELDLIST_H
#define FIELDLIST_H
#include <typetable.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct FieldList {
    char *name;
    int fieldIndex;
    struct TypeTable *type;
    struct FieldList *next;
} FieldList;

extern struct FieldList* head;
extern int fieldIndexCounter;

struct FieldList* FieldListAppend(char* name, struct TypeTable* type);
struct FieldList* FieldListReturnHead();
struct FieldList* FieldListSetHead(struct FieldList* newHead);

#endif