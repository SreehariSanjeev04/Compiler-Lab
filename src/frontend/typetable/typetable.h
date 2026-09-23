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


/**
 * Initializes the TypeTable with the predefined types: int, bool, str, void.
 */
void TypeTableCreate();
/**
 * Destroys the TypeTable and frees all allocated memory.
 */
void TypeTableDestroy();
/**
 * Looks up a type by name.
 * @return the TypeTable entry, or NULL if not found
 */
struct TypeTable* TLookup(char *name);
/**
 * Installs a new type.
 * @return the new entry, or NULL if the type already exists or allocation fails
 */
struct TypeTable* TInstall(char *name, int size, struct FieldList *fields);
/**
 * @return the head of the TypeTable
 */
struct TypeTable* GetTypeTableHead();
/**
 * Looks up a field of a type by name.
 * @return the FieldList entry, or NULL if not found
 */
struct FieldList* FLookup(TypeTable* type, char* name);
int GetSize(TypeTable* type);

#endif