#include <gsymboltable_utils.h>
#include <gsymboltable.h>
#include <constants.h>
#include <dimnode.h>

// Set from the compiler command line (-g); gates printGlobalSymbolTable().
int showGlobalSymbolTable = 0;

/**
 * This function frees the memory allocated for the symbol table and its associated structures.
 * It traverses the linked list of symbols and deallocates memory for each symbol's name
 * and dimension sizes, as well as the symbol itself. After freeing all symbols, it sets the head of the symbol table to NULL.
 * @return: void
 */
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

/**
 * This function creates a stride array for a given symbol representing an array variable.
 * The stride array is used to calculate the memory address of elements in multi-dimensional arrays.
 * @param symbol: The symbol table entry for the array variable.
 * @return: A pointer to the dynamically allocated stride array.
 */
int* createStrideArray(struct Gsymbol* symbol) {
    if (symbol == NULL || symbol->dimensions <= 0) {
        fprintf(stderr, "Error: Invalid symbol or dimensions\n");
        exit(1);
    }

    int* strideArray = (int*)malloc(symbol->dimensions * sizeof(int));
    if (strideArray == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for stride array\n");
        exit(1);
    }

    // dimension_sizes is stored in declaration order (outermost first).
    // Row-major strides: stride[k] = product of sizes of dimensions k+1..
    int* sizes = (int*)malloc(symbol->dimensions * sizeof(int));
    if (sizes == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for dimension sizes\n");
        free(strideArray);
        exit(1);
    }
    struct dimension_sizes* current = symbol->dimension_sizes;
    for (int k = 0; k < symbol->dimensions; k++) {
        if (current == NULL) {
            fprintf(stderr, "Error: Dimension sizes do not match the number of dimensions\n");
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

/**
 * This function handles the dimension sizes for a given symbol representing an array variable.
 * It traverses the linked list of dimension sizes and adds each size to the symbol's dimension
 * sizes linked list. It also updates the number of dimensions and the total size of the array.
 * @param symbol: The symbol table entry for the array variable.
 * @param DimNode: The head of the linked list containing the sizes of each dimension.
 * @return: void
 */
void handleDimensionSizes(struct Gsymbol* symbol, struct DimNode* DimNode) {
    if (symbol == NULL || DimNode == NULL || GLookup(symbol->name) == NULL) {
        fprintf(stderr, "Error: Invalid symbol or DimNode\n");
        exit(1);
    }

    struct DimNode* currentDim = DimNode;
    while (currentDim != NULL) {
        addDimensionSizes(symbol, currentDim->size);
        currentDim = currentDim->next;
    }
}

/**
 * This function adds a new dimension size to the specified symbol representing an array variable.
 * It updates the linked list of dimension sizes, increments the number of dimensions, and adjusts the total size of the array.
 * @param symbol: The symbol table entry for the array variable.
 * @param size: The size of the new dimension to be added.
 * @return: void
 */
void addDimensionSizes(struct Gsymbol* symbol, int size) {
    if(symbol == NULL) {
        fprintf(stderr, "Error: Symbol is NULL\n");
        exit(1);
    }
    if(size <= 0) {
        fprintf(stderr, "Error: Dimension size must be positive, got %d\n", size);
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
        fprintf(stderr, "Error: Array size exceeds maximum allowed address space\n");
        exit(1);
    }
}

/**
 * This function assigns sequential static binding addresses to all symbols in
 * the symbol table. Scalars occupy one word; arrays occupy one word per element.
 * It must be called once, after all declarations have been parsed.
 * @return: void
 */
void assignBindingAddresses(void) {
    currentBindingAddress = DEFAULT_BINDING_ADDRESS;
    struct Gsymbol* current = head;
    while (current != NULL) {
        if (current->paramList != NULL) {
            current = current->next;
            continue; // Skip function symbols
        }
        current->binding = currentBindingAddress;
        if(current->size > MAX_ARRAY_ADDRESS) {
            fprintf(stderr, "Error: Variable '%s' requires more memory than available (%d)\n", current->name, current->size);
            exit(1);
        }
        currentBindingAddress += current->size;
        current = current->next;
    }
}

void printGlobalSymbolTable(void) {
    struct Gsymbol* current = head;
    printf("Global Symbol Table:\n");
    printf("Name\tType\tPointerLevel\tSize\tBinding\tDimensions\tDimension Sizes\n");
    while (current != NULL) {
        printf("%s\t%d\t%d\t%d\t%d\t%d\t", current->name, current->type, current->pointerLevel, current->size, current->binding, current->dimensions);
        struct dimension_sizes* dim_current = current->dimension_sizes;
        while (dim_current != NULL) {
            printf("%d ", dim_current->size);
            dim_current = dim_current->next;
        }
        printf("\n");
        if(current->paramList != NULL) {
            printf("  Parameters: ");
            struct ParamList* param_current = current->paramList;
            while (param_current != NULL) {
                printf("%s:%d ", param_current->name, param_current->type);
                param_current = param_current->next;
            }
            printf("\n");
        }
        current = current->next;
    }
}
