#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dimension_sizes {
    int size;
    struct dimension_sizes* next;
} dimension_sizes;

typedef struct TypeRef {
    int type; // Type of the variable (eg. TYPE_INT, TYPE_BOOL, etc.)
    int pointerLevel; // Level of pointer indirection (0 for non-pointer types)
} TypeRef;

typedef struct Gsymbol {
    char* name;       
    TypeRef typeRef; // Type reference for the variable
    int size;         // size of the type of the variable
    int binding;      // stores the static memory address allocated to the variable
    int dimensions;    // number of dimensions for arrays
    struct dimension_sizes* dimension_sizes; // linked list to hold the sizes of each dimension for arrays
    struct Gsymbol *next;
} Gsymbol;

extern struct Gsymbol *head;
extern int currentBindingAddress;

struct Gsymbol* Lookup(char *name);
struct Gsymbol* Install(char *name, int type, int pointerLevel);

#endif // SYMBOLTABLE_H