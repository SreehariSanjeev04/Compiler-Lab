#include "logger.h"
#include <gsymboltable_utils.h>
#include <gsymboltable.h>
#include <constants.h>
#include <dimnode.h>
#include <tupletable.h>

// Compiler flag to display the global symbol table
int showGlobalSymbolTable = 0;

void freeSymbolTable(void) {
    struct Gsymbol* current = head;
    while (current != NULL) {
        struct Gsymbol* temp = current;
        current = current->next;
        free(temp->name);
        if (temp->dimension_sizes != NULL) {
            struct dimension_sizes* dim_current = temp->dimension_sizes;
            while (dim_current != NULL) {
                struct dimension_sizes* dim_temp = dim_current;
                dim_current = dim_current->next;
                free(dim_temp);
            }
        }
        free(temp);
    }
    head = NULL;
}

int* createStrideArray(struct Gsymbol* symbol) {
    if (symbol == NULL || symbol->dimensions <= 0) {
        LOG_ERROR("Error: Invalid symbol or dimensions\n");
        exit(1);
    }

    int* strideArray = (int*)malloc(symbol->dimensions * sizeof(int));
    if (strideArray == NULL) {
        LOG_ERROR("Error: Memory allocation failed for stride array\n");
        exit(1);
    }

    // dimension_sizes is stored in declaration order (outermost first).
    // Row-major strides: stride[k] = product of sizes of dimensions k+1..
    int* sizes = (int*)malloc(symbol->dimensions * sizeof(int));
    if (sizes == NULL) {
        LOG_ERROR("Error: Memory allocation failed for dimension sizes\n");
        free(strideArray);
        exit(1);
    }
    struct dimension_sizes* current = symbol->dimension_sizes;
    for (int k = 0; k < symbol->dimensions; k++) {
        if (current == NULL) {
            LOG_ERROR("Error: Dimension sizes do not match the number of dimensions\n");
            free(strideArray);
            free(sizes);
            exit(1);
        }
        sizes[k] = current->size;
        current = current->next;
    }

    strideArray[symbol->dimensions - 1] = 1;
    for (int k = symbol->dimensions - 2; k >= 0; k--) {
        strideArray[k] = sizes[k + 1] * strideArray[k + 1];
    }
    free(sizes);

    return strideArray;
}

void handleDimensionSizes(struct Gsymbol* symbol, struct DimNode* DimNode) {
    if (symbol == NULL || DimNode == NULL || GLookup(symbol->name) == NULL) {
        LOG_ERROR("Error: Invalid symbol or DimNode\n");
        exit(1);
    }

    struct DimNode* currentDim = DimNode;
    while (currentDim != NULL) {
        addDimensionSizes(symbol, currentDim->size);
        currentDim = currentDim->next;
    }
}

void addDimensionSizes(struct Gsymbol* symbol, int size) {
    if(symbol == NULL) {
        LOG_ERROR("Error: Symbol is NULL\n");
        exit(1);
    }
    if(size <= 0) {
        LOG_ERROR("Error: Dimension size must be positive, got %d\n", size);
        exit(1);
    }
    if (symbol->dimension_sizes == NULL) {
        symbol->dimension_sizes = (struct dimension_sizes*)malloc(sizeof(struct dimension_sizes));
        symbol->dimension_sizes->size = size;
        symbol->dimension_sizes->next = NULL;
    } else {
        struct dimension_sizes* newDim = (struct dimension_sizes*)malloc(sizeof(struct dimension_sizes));
        newDim->size = size;
        struct dimension_sizes* current = symbol->dimension_sizes;
        while (current->next != NULL) {
            current = current->next;
        } 
        current->next = newDim;
    }
    symbol->dimensions += 1;
    symbol->pointerLevel = symbol->dimensions;
    symbol->size *= size;
    if(symbol->size > MAX_ARRAY_ADDRESS) {
        LOG_ERROR("Error: Array size exceeds maximum allowed address space\n");
        exit(1);
    }
}

void assignBindingAddresses(void) {
    currentBindingAddress = DEFAULT_BINDING_ADDRESS;
    struct Gsymbol* current = head;
    while (current != NULL) {
        if (current->paramList != NULL) {
            current = current->next;
            continue;
        }
        current->binding = currentBindingAddress;
        if(current->size > MAX_ARRAY_ADDRESS) {
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
    LOG_INFO("Name\tType\tPointerLevel\tSize\tBinding\tDimensions");
    while (current != NULL) {
        LOG_INFO("%s\t%d\t%d\t%d\t%d\t%d", current->name, current->type, current->pointerLevel, current->size, current->binding, current->dimensions);
        struct dimension_sizes* dim_current = current->dimension_sizes;
        while (dim_current != NULL) {
            LOG_INFO("  Dimension size: %d", dim_current->size);
            dim_current = dim_current->next;
        }
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
