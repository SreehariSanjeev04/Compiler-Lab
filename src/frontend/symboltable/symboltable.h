#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dimension_sizes {
    int size;
    struct dimension_sizes* next;
} dimension_sizes;

typedef struct Gsymbol {
    char* name;       
    int type;         // type of the variable
    int size;         // size of the type of the variable
    int binding;      // stores the static memory address allocated to the variable
    int dimensions;    // number of dimensions for arrays
    struct dimension_sizes* dimension_sizes; // linked list to hold the sizes of each dimension for arrays
    struct Gsymbol *next;
} Gsymbol;

extern struct Gsymbol *head;
struct Gsymbol* Lookup(char *name);
struct Gsymbol* Install(char *name, int type);

#endif // SYMBOLTABLE_H