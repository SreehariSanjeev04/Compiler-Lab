#include "logger.h"
#include <gsymboltable_utils.h>
#include <gsymboltable.h>
#include <constants.h>
#include <fieldlist.h>

// Compiler flag to display the global symbol table
int showGlobalSymbolTable = 0;

void freeSymbolTable(void) {
    struct Gsymbol* current = head;
    while (current != NULL) {
        struct Gsymbol* temp = current;
        current = current->next;
        free(temp->name);
        free(temp);
    }
    head = NULL;
}

void setArrayLength(struct Gsymbol* symbol, int length) {
    if(symbol == NULL) {
        LOG_ERROR("Error: Symbol is NULL\n");
        exit(1);
    }
    if(length <= 0) {
        LOG_ERROR("Error: Array length must be positive, got %d\n", length);
        exit(1);
    }
    symbol->arrayLength = length;
    symbol->pointerLevel = 1;
    symbol->size = length;
    if(DEFAULT_BINDING_ADDRESS + symbol->size > MAX_ARRAY_ADDRESS) {
        LOG_ERROR("Error: Array size exceeds maximum allowed address space\n");
        exit(1);
    }
}

void assignBindingAddresses(void) {
    currentBindingAddress = DEFAULT_BINDING_ADDRESS;
    struct Gsymbol* current = head;
    while (current != NULL) {
        // function
        if (current->flabel >= 0) {
            current = current->next;
            continue;
        }
        current->binding = currentBindingAddress;
        if(currentBindingAddress + current->size > MAX_ARRAY_ADDRESS) {
            LOG_ERROR("Error: Variable '%s' requires more memory than available (%d)\n", current->name, current->size);
            exit(1);
        }
        currentBindingAddress += current->size;
        current = current->next;
    }
}

void printGlobalSymbolTable(void) {
    struct Gsymbol* current = head;
    LOG_INFO("Global Symbol Table:");
    LOG_INFO("Name\tType\tPointerLevel\tSize\tBinding\tArrayLength");
    while (current != NULL) {
        LOG_INFO("%s\t%d\t%d\t%d\t%d\t%d", current->name, GTypeKind(current), current->pointerLevel, current->size, current->binding, current->arrayLength);
        if(current->paramList != NULL) {
            struct ParamList* param_current = current->paramList;
            while (param_current != NULL) {
                LOG_INFO("  Parameter: %s:%d", param_current->name, param_current->type);
                param_current = param_current->next;
            }
        }
        current = current->next;
    }
}
