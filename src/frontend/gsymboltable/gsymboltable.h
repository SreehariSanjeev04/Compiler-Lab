#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paramlist.h>
#include <typetable.h>
#include <classtable.h>

typedef struct Gsymbol {
    char* name;
    struct TypeTable* type; // pointer to the type table, contains default types as well
    struct ClassTable* cPtr; // pointer to the class if the variable is a class
    int pointerLevel; // level of pointer, 0 for scalars
    int size;         // size of the type of the variable
    int binding;      // stores the static memory address allocated to the variable
    int arrayLength;   // zero for non-arrays
    int flabel;        // label number identifying the start of a function's code
    struct ParamList* paramList; // linked list to hold the parameters for functions
    struct Gsymbol *next;
} Gsymbol;

extern struct Gsymbol *head;
extern int currentBindingAddress;

/**
 * Looks up a variable in the symbol table by name.
 * @return the Gsymbol entry if found, NULL otherwise
 */
struct Gsymbol* GLookup(const char *name);
/**
 * Installs a new variable into the symbol table.
 * @return the newly created Gsymbol entry
 */
struct Gsymbol* GInstall(const char *name, struct TypeTable* type,
                         struct ClassTable* cPtr, int pointerLevel);

int GTypeKind(const struct Gsymbol* symbol);

#endif // SYMBOLTABLE_H
