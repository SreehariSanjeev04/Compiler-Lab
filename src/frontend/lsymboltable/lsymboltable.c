#include "lsymboltable.h"

struct Lsymbol* head = NULL;

struct Lsymbol* LInstall(char* name, int type, int binding) {
    struct Lsymbol* newSymbol = (struct Lsymbol*)malloc(sizeof(struct Lsymbol));
    if(!newSymbol) {
        fprintf(stderr, "Memory allocation failed for Lsymbol\n");
        return NULL;
    }
    newSymbol->name = name;
    newSymbol->type = type;
    newSymbol->binding = binding;
    newSymbol->next = NULL;

    if(head == NULL) {
        head = newSymbol;
    } else {
        struct Lsymbol* current = head;
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = newSymbol;
    }
    return newSymbol;
}

struct Lsymbol* LLookup(char* name) {
    struct Lsymbol* current = head;
    while(current != NULL) {
        if(strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}