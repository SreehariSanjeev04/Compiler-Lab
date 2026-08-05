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
    if(atoi(symbol->name) < 0) {
        fprintf(stderr, "Error: Dimension size cannot be negative\n");
        exit(1);
    }
    if (symbol->dimension_sizes == NULL) {
        symbol->dimension_sizes = (struct dimension_sizes*)malloc(sizeof(struct dimension_sizes));
        symbol->dimension_sizes->size = dimension;
        symbol->dimension_sizes->next = NULL;
    } else {
        struct dimension_sizes* current = symbol->dimension_sizes;
        current->next = (struct dimension_sizes*)malloc(sizeof(struct dimension_sizes));
        current->next->size = dimension;
        current->next->next = NULL;
    }
    symbol->dimensions += 1;
    symbol->size *= dimension;
    if(symbol->size > MAX_ARRAY_ADDRESS) {
        fprintf(stderr, "Error: Array size exceeds maximum allowed address space\n");
        exit(1);
    }
    currentBindingAddress += dimension;
}