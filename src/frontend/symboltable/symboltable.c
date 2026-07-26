#include <symboltable.h>
#include <constants.h>

struct Gsymbol* head = NULL;
int currentBindingAddress = DEFAULT_BINDING_ADDRESS;

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
    // printf("Installing variable: %s of type %d\n", name, type);
    if (name == NULL || strlen(name) == 0) {
        fprintf(stderr, "Error: Variable name is invalid. Only non-empty names are allowed.\n");
        exit(1);
    }
    if (Lookup(name) != NULL) {
        fprintf(stderr, "Error: Variable %s already declared\n", name);
        exit(1);
    }
    if(currentBindingAddress + size >= 4121) {
        fprintf(stderr, "Error: Memory limit exceeded. Cannot allocate more variables.\n");
        exit(1);
    }
    struct Gsymbol* newSymbol = (struct Gsymbol*)malloc(sizeof(struct Gsymbol));
    newSymbol->name = (char*)malloc(strlen(name) + 1);
    strcpy(newSymbol->name, name);
    newSymbol->type = type;
    newSymbol->size = size;
    newSymbol->binding = currentBindingAddress += size;
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