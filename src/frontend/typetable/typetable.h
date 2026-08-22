#ifndef TYPETABLE_H
#define TYPETABLE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct TypeTable {
    char *name;
    int size;
    struct TypeTable *next;
    struct FieldList *fields;
} TypeTable;


void TypeTableCreate();
void TypeTableDestroy();
struct TypeTable* TLookup(char *name);
struct TypeTable* TInstall(char *name, int size, struct FieldList *fields);
struct TypeTable* GetTypeTableHead();
struct FieldList* FLookup(TypeTable* type, char* name);
int GetSize(TypeTable* type);

#endif