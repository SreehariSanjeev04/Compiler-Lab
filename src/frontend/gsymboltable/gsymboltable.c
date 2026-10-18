#include "logger.h"
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

struct Gsymbol* GInstall(const char *name, struct TypeTable* type,
                         struct ClassTable* cPtr, int pointerLevel) {
    if (name == NULL || strlen(name) == 0) {
        LOG_ERROR("Error: Variable name is invalid. Only non-empty names are allowed.\n");
        exit(1);
    }
    if (GLookup(name) != NULL) {
        LOG_ERROR("Error: Variable %s already declared\n", name);
        exit(1);
    }

    if ((type == NULL) == (cPtr == NULL)) {
        LOG_ERROR("Exactly one of type or class must be set.\n");
        exit(1);
    }
    // LOG_INFO("Global Install %s -> %d -> %d", name, type, pointerLevel);
    struct Gsymbol* newSymbol = (struct Gsymbol*)malloc(sizeof(struct Gsymbol));
    newSymbol->name = (char*)malloc(strlen(name) + 1);
    strcpy(newSymbol->name, name);
    newSymbol->type = type;
    newSymbol->pointerLevel = pointerLevel;
    newSymbol->size = DEFAULT_VAR_SIZE;
    newSymbol->binding = -1; // Assigned by assignBindingAddresses() after all declarations
    newSymbol->next = NULL;
    newSymbol->arrayLength = 0;
    newSymbol->flabel = 0;
    newSymbol->paramList = NULL;
    newSymbol->cPtr = cPtr;
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

int GTypeKind(const struct Gsymbol* symbol) {
    if (symbol->cPtr != NULL) return TYPE_CLASS;
    if (strcmp(symbol->type->name, "int") == 0) return TYPE_INT;
    if (strcmp(symbol->type->name, "bool") == 0) return TYPE_BOOL;
    if (strcmp(symbol->type->name, "str") == 0) return TYPE_STRING;
    if (strcmp(symbol->type->name, "void") == 0) return TYPE_VOID;
    return TYPE_CUSTOM;
}
