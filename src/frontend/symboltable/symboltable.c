#include <symboltable.h>
#include <constants.h>

struct Gsymbol* head = NULL;
int currentBindingAddress = DEFAULT_BINDING_ADDRESS;

/**
 * This function looks up a variable in the symbol table by its name.
 * @param name: The name of the variable to look up
 * @return: A pointer to the Gsymbol structure if found, NULL otherwise
 */
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

/**
 * This function installs a new variable into the symbol table.
 * @param name: The name of the variable to be installed
 * @param type: The type of the variable (e.g., TYPE_INT, TYPE_BOOL, etc.)
 * @return: A pointer to the newly created Gsymbol structure for the variable
 */
struct Gsymbol* Install(char *name, int type, int pointerLevel) {
    if (name == NULL || strlen(name) == 0) {
        fprintf(stderr, "Error: Variable name is invalid. Only non-empty names are allowed.\n");
        exit(1);
    }
    if (Lookup(name) != NULL) {
        fprintf(stderr, "Error: Variable %s already declared\n", name);
        exit(1);
    }
    struct Gsymbol* newSymbol = (struct Gsymbol*)malloc(sizeof(struct Gsymbol));
    newSymbol->name = (char*)malloc(strlen(name) + 1);
    strcpy(newSymbol->name, name);
    newSymbol->type = type;
    newSymbol->pointerLevel = pointerLevel;
    newSymbol->size = 1; // default size
    newSymbol->binding = -1; // Assigned by assignBindingAddresses() after all declarations
    newSymbol->next = NULL;
    newSymbol->dimensions = 0;
    newSymbol->dimension_sizes = NULL;

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