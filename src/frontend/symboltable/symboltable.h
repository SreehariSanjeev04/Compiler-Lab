#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Gsymbol {
    char* name;       
    int type;         // type of the variable
    int size;         // size of the type of the variable
    int binding;      // stores the static memory address allocated to the variable
    struct Gsymbol *next;
} Gsymbol;

struct Gsymbol* Lookup(char *name);
struct Gsymbol* Install(char *name, int type, int size);

#endif // SYMBOLTABLE_H