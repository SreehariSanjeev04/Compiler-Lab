#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paramlist.h>

typedef struct dimension_sizes {
    int size;
    struct dimension_sizes* next;
} dimension_sizes;

typedef struct Gsymbol {
    char* name;
    int type;         // type of the variable (e.g., TYPE_INT, TYPE_BOOL, etc.)
    int pointerLevel; // level of pointer indirection (0 for non-pointer, 1 for single pointer, etc.)
    int size;         // size of the type of the variable
    int binding;      // stores the static memory address allocated to the variable
    int dimensions;    // number of dimensions for arrays
    struct dimension_sizes* dimension_sizes; // linked list to hold the sizes of each dimension for arrays
    int flabel;        // label number (F0, F1, ...) identifying the start of a function's code
    struct ParamList* paramList; // linked list to hold the parameters for functions
    struct Gsymbol *next;
} Gsymbol;

extern struct Gsymbol *head;
extern int currentBindingAddress;

struct Gsymbol* GLookup(const char *name);
struct Gsymbol* GInstall(const char *name, int type, int pointerLevel);

#endif // SYMBOLTABLE_H