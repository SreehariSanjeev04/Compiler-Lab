#ifndef LSYMBOLTABLE_H
#define LSYMBOLTABLE_H
#include <stdio.h>
#include <stdlib.h>

typedef struct Lsymbol {
    char* name; // name of the variable
    int type;         // type of the variable (e.g., TYPE_INT, TYPE_BOOL, etc.)
    int binding;      // BP-relative offset: params are negative (-3, -4, ...), locals positive (+1, +2, ...)
    int pointerLevel; // Pointer level of the variable
    struct Lsymbol *next; // pointer to the next symbol in the list
} Lsymbol;

extern struct Lsymbol* Lhead;

struct Lsymbol* LInstall(char* name, int type, int pointerLevel, int binding);
struct Lsymbol* LLookup(char* name);
struct Lsymbol* LSymbolGetHead();
void LSymbolReset();
void assignLocalBindingAddresses(void);
void LSymbolTableDestroy(struct Lsymbol* head);

#endif // LSYMBOLTABLE_H