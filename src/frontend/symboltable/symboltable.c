#include <symboltable.h>

struct Gsymbol* head = NULL;
int currentBindingAddress = 4096;

struct Gsymbol* Lookup(char *name) {
    struct Gsymbol* current = head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct Gsymbol* Install(char *name, int type, int size) {
    if (Lookup(name) != NULL) {
        fprintf(stderr, "Error: Variable %s already declared\n", name);
        exit(1);
    }
    struct Gsymbol* newSymbol = (struct Gsymbol*)malloc(sizeof(struct Gsymbol));
    newSymbol->name = (char*)malloc(strlen(name) + 1);
    strcpy(newSymbol->name, name);
    newSymbol->type = type;
    newSymbol->size = size;
    newSymbol->binding = currentBindingAddress++;
    newSymbol->next = NULL;

    if (head == NULL) {
        head = newSymbol;
    } else {
        struct Gsymbol* current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newSymbol;
    }

    return newSymbol;
}