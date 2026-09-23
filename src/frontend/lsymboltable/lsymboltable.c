#include "lsymboltable.h"
#include <string.h>

struct Lsymbol* Lhead = NULL;
struct Lsymbol* Ltail = NULL;

struct Lsymbol* LInstall(char* name, int type, int pointerLevel, int binding) {
    struct Lsymbol* newSymbol = (struct Lsymbol*)malloc(sizeof(struct Lsymbol));
    if(!newSymbol) {
        fprintf(stderr, "Memory allocation failed for Lsymbol\n");
        return NULL;
    }
    if(LLookup(name) != NULL) {
        fprintf(stderr, "Error: Variable '%s' is already defined in the local symbol table.\n", name);
        free(newSymbol);
        return NULL;
    }
    newSymbol->name = strdup(name);
    newSymbol->type = type;
    newSymbol->binding = binding;
    newSymbol->pointerLevel = pointerLevel; 
    newSymbol->next = NULL;

    if(Lhead == NULL) {
        Lhead = Ltail = newSymbol;
    } else {
        Ltail->next = newSymbol;
        Ltail = newSymbol;
    }
    return newSymbol;
}


struct Lsymbol* LLookup(char* name) {
    struct Lsymbol* current = Lhead;
    while(current != NULL) {
        if(strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct Lsymbol* LSymbolGetHead() {
    return Lhead;
}

void LSymbolReset() {
    Lhead = NULL;
    Ltail = NULL;
}

void LSymbolTableDestroy(struct Lsymbol* head) {
    struct Lsymbol* current = head;
    struct Lsymbol* nextSymbol;

    while(current != NULL) {
        nextSymbol = current->next;
        free(current->name);
        free(current);
        current = nextSymbol;
    }
    LSymbolReset();
}

void assignLocalBindingAddresses(void) {
    int binding = 0;
    struct Lsymbol* current = Lhead;
    while (current != NULL) {
        if (current->binding == 0) {
            current->binding = ++binding;
        }
        current = current->next;
    }
}