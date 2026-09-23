#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paramlist.h>
#include <tupletable.h>

typedef struct dimension_sizes {
    int size;
    struct dimension_sizes* next;
} dimension_sizes;

typedef struct Gsymbol {
    char* name;
    int type;         // type of the variable
    int pointerLevel; // level of pointer, 0 for scalars
    int size;         // size of the type of the variable
    int binding;      // stores the static memory address allocated to the variable
    int dimensions;    // number of dimensions for arrays
    struct dimension_sizes* dimension_sizes; // linked list to hold the sizes of each dimension for arrays
    int flabel;        // label number identifying the start of a function's code
    struct ParamList* paramList; // linked list to hold the parameters for functions
    struct TupleTable* tupleEntry; // linked list to hold the tuple details
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
struct Gsymbol* GInstall(const char *name, int type, int pointerLevel);

#endif // SYMBOLTABLE_H