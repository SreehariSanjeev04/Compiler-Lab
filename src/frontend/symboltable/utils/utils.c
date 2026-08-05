#include <utils.h>
#include <symboltable.h>

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
}