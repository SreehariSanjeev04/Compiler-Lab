#include <gsymboltable.h>
#include <constants.h>

struct Gsymbol* head = NULL;
int currentBindingAddress = DEFAULT_BINDING_ADDRESS;

struct Gsymbol* GLookup(const char *name) {
    struct Gsymbol* current = head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct Gsymbol* GInstall(const char *name, int type, int pointerLevel) {
    if (name == NULL || strlen(name) == 0) {
        fprintf(stderr, "Error: Variable name is invalid. Only non-empty names are allowed.\n");
        exit(1);
    }
    if (GLookup(name) != NULL) {
        fprintf(stderr, "Error: Variable %s already declared\n", name);
        exit(1);
    }
    struct Gsymbol* newSymbol = (struct Gsymbol*)malloc(sizeof(struct Gsymbol));
    newSymbol->name = (char*)malloc(strlen(name) + 1);
    strcpy(newSymbol->name, name);
    newSymbol->type = type;
    newSymbol->pointerLevel = pointerLevel;
    newSymbol->size = DEFAULT_VAR_SIZE; // scalars occupy one word; arrays grow via addDimensionSizes()
    newSymbol->binding = -1; // Assigned by assignBindingAddresses() after all declarations
    newSymbol->next = NULL;
    newSymbol->dimensions = 0;
    newSymbol->dimension_sizes = NULL;
    newSymbol->flabel = 0;
    newSymbol->paramList = NULL;
    newSymbol->tupleEntry = NULL;
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