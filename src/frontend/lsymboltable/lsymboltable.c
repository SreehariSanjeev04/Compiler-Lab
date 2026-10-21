#include "logger.h"
#include "lsymboltable.h"
#include <string.h>
#include <constants.h>

struct Lsymbol* Lhead = NULL;
struct Lsymbol* Ltail = NULL;

struct Lsymbol* LInstall(char* name, struct TypeTable* type, int pointerLevel, int binding, struct ClassTable* cptr) {
    struct Lsymbol* newSymbol = (struct Lsymbol*)malloc(sizeof(struct Lsymbol));
    if(!newSymbol) {
        LOG_ERROR("Memory allocation failed for Lsymbol\n");
        return NULL;
    }
    if(LLookup(name) != NULL) {
        LOG_ERROR("Error: Variable '%s' is already defined in the local symbol table.\n", name);
        free(newSymbol);
        exit(1);
    }
    newSymbol->name = strdup(name);
    newSymbol->typeEntry = type;
    newSymbol->cPtr = cptr;
    if (newSymbol->cPtr != NULL) newSymbol->type = TYPE_CLASS;
    else newSymbol->type = strcmp(type->name, "int") == 0 ? TYPE_INT :
                      strcmp(type->name, "str") == 0 ? TYPE_STRING :
                      strcmp(type->name, "bool") == 0 ? TYPE_BOOL : TYPE_CUSTOM;
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

void LSymbolAssignBindingAddresses(void) {
    int binding = 0;
    struct Lsymbol* current = Lhead;
    while (current != NULL) {
        if (current->binding == 0) {
            // locally declared variables
            current->binding = binding + 1;
            binding++;
        }
        current = current->next;
    }
}
