#ifndef FIELDLIST_H
#define FIELDLIST_H
#include <typetable.h>

typedef struct FieldList {
    char *name;
    int fieldIndex;
    struct TypeTable *type;
    struct FieldList *next;
} FieldList;

#endif