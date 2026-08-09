#include <utils.h>
#include <symboltable.h>
#include <constants.h>

/**
 * This function frees the memory allocated for the symbol table and its associated structures.
 * It traverses the linked list of symbols and deallocates memory for each symbol's name
 * and dimension sizes, as well as the symbol itself. After freeing all symbols, it sets the head of the symbol table to NULL.
 * @return: void
 */
void freeSymbolTable() {
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

    struct dimension_sizes* current = symbol->dimension_sizes;
    int totalSize = 1;
    for (int i = symbol->dimensions - 1; i >= 0; i--) {
        if (current == NULL) {
            fprintf(stderr, "Error: Dimension sizes do not match the number of dimensions\n");
            free(strideArray);
            exit(1);
        }
        strideArray[i] = totalSize;
        totalSize *= current->size;
        current = current->next;
    }

    return strideArray;
}

/**
 * This function adds a new dimension size to the specified symbol representing an array variable.
 * It updates the linked list of dimension sizes, increments the number of dimensions, and adjusts the total size of the array.
 * @param symbol: The symbol table entry for the array variable.
 * @param dimension: The size of the new dimension to be added.
 * @return: void
 * @details: The dimension sizes are filled in the reverse order of the dimensions. For example, for a 2D array declared as int arr[3][4], the first call to addDimensionSizes will add 4, and the second call will add 3.
 */
void addDimensionSizes(struct Gsymbol* symbol, int dimension) {
    if(symbol == NULL) {
        fprintf(stderr, "Error: Symbol is NULL\n");
        exit(1);
    }
    if(dimension <= 0) {
        fprintf(stderr, "Error: Dimension size must be positive, got %d\n", dimension);
        exit(1);
    }
    if (symbol->dimension_sizes == NULL) {
        symbol->dimension_sizes = (struct dimension_sizes*)malloc(sizeof(struct dimension_sizes));
        symbol->dimension_sizes->size = dimension;
        symbol->dimension_sizes->next = NULL;
    } else {
        // Insert at the head so that the sizes are stored in the reverse order of
        // the declaration (last dimension first), as expected by createStrideArray.
        struct dimension_sizes* newDim = (struct dimension_sizes*)malloc(sizeof(struct dimension_sizes));
        newDim->size = dimension;
        newDim->next = symbol->dimension_sizes;
        symbol->dimension_sizes = newDim;
    }
    symbol->dimensions += 1;
    symbol->size *= dimension;
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
void assignBindingAddresses() {
    currentBindingAddress = DEFAULT_BINDING_ADDRESS;
    struct Gsymbol* current = head;
    while (current != NULL) {
        current->binding = currentBindingAddress;
        if(current->size > MAX_ARRAY_ADDRESS) {
            fprintf(stderr, "Error: Variable '%s' requires more memory than available (%d)\n", current->name, current->size);
            exit(1);
        }
        currentBindingAddress += current->size;
        current = current->next;
    }
}

/*
 * This function returns the pointer type corresponding to a given base type.
 * @param baseType: The base type for which to return a pointer type.
 * @return: The corresponding pointer type.
 */
int returnPointerType(int baseType) {
    switch(baseType) {
        case TYPE_INT:
            return TYPE_POINTER_INT;
        case TYPE_BOOL:
            return TYPE_POINTER_BOOL;
        case TYPE_STRING:
            return TYPE_POINTER_STRING;
        default:
            fprintf(stderr, "Error: Invalid base type for pointer conversion\n");
            exit(1);
    }
}

/*
 * This function checks if a given type is a pointer type.
 * @param type: The type to check.
 * @return: true if the type is a pointer type, false otherwise.
 */
bool isPointerType(int type) {
    return (type == TYPE_POINTER_INT || type == TYPE_POINTER_BOOL || type == TYPE_POINTER_STRING);
}

int returnBaseType(int pointerType) {
    switch(pointerType) {
        case TYPE_POINTER_INT:
            return TYPE_INT;
        case TYPE_POINTER_BOOL:
            return TYPE_BOOL;
        case TYPE_POINTER_STRING:
            return TYPE_STRING;
        default:
            fprintf(stderr, "Error: Invalid pointer type for base type conversion\n");
            exit(1);
    }
}

/**
 * This function returns the pointer level of a given symbol.
 * @param symbol: The symbol table entry for which to return the pointer level.
 * @return: The pointer level of the symbol.
 */
int returnPointerLevel(struct Gsymbol* symbol) {
    if(symbol == NULL) {
        fprintf(stderr, "Error: Symbol is NULL\n");
        exit(1);
    }
    if(!isPointerType(symbol->typeRef.type)) {
        return 0; // Not a pointer type
    }
    return symbol->typeRef.pointerLevel;
}