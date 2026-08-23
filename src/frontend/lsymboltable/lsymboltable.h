#ifndef LSYMBOLTABLE_H
#define LSYMBOLTABLE_H
#include <stdio.h>
#include <stdlib.h>

typedef struct Lsymbol {
    char* name; // name of the variable
    int type;         // type of the variable (e.g., TYPE_INT, TYPE_BOOL, etc.)
    int binding;      // stores the static memory address allocated to the variable
    struct Lsymbol *next; // pointer to the next symbol in the list
} Lsymbol;

extern struct Lsymbol* head;

struct Lsymbol* LInstall(char* name, int type, int binding);
struct Lsymbol* LLookup(char* name);

#endif