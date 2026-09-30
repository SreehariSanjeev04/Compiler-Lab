#include "logger.h"
#include "lsymboltable.h"
#include <string.h>

struct Lsymbol* Lhead = NULL;
struct Lsymbol* Ltail = NULL;

struct Lsymbol* LInstall(char* name, int type, int pointerLevel, int binding) {
    struct Lsymbol* newSymbol = (struct Lsymbol*)malloc(sizeof(struct Lsymbol));
    if(!newSymbol) {
        LOG_ERROR("Memory allocation failed for Lsymbol\n");
        return NULL;
    }
    if(LLookup(name) != NULL) {
        LOG_ERROR("Error: Variable '%s' is already defined in the local symbol table.\n", name);
        free(newSymbol);
        return NULL;
    }
    // LOG_INFO("Local install %s -> %d\n", name, type);
    newSymbol->name = strdup(name);
    newSymbol->type = type;
    newSymbol->binding = binding;
    newSymbol->pointerLevel = pointerLevel; 
    newSymbol->tupleEntry = NULL;
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

void LSymbolAssignBindingAddresses(void) {
    int binding = 0;
    struct Lsymbol* current = Lhead;
    while (current != NULL) {
        if (current->binding == 0) {
            current->binding = binding + 1;
            binding += (current->tupleEntry != NULL && current->pointerLevel == 0)
                       ? current->tupleEntry->size : 1;
        }
        current = current->next;
    }
}
