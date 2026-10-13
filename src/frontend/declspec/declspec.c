#include "declspec.h"

#include <stdlib.h>

#include <constants.h>
#include <flabel.h>
#include <gsymboltable.h>
#include <gsymboltable_utils.h>
#include <logger.h>
#include <lsymboltable.h>

DeclType* makeDeclType(int kind, struct TypeTable* table) {
    DeclType* type = malloc(sizeof(*type));
    if (type == NULL) { LOG_ERROR("Out of memory\n"); exit(1); }
    *type = (DeclType){kind, table};
    return type;
}

Declarator* makeDeclarator(char* name, DeclaratorKind kind, int pointerLevel,
                            int arrayLength, struct ParamList* parameters) {
    Declarator* decl = malloc(sizeof(*decl));
    if (decl == NULL) { LOG_ERROR("Out of memory\n"); exit(1); }
    *decl = (Declarator){name, kind, pointerLevel, arrayLength, parameters, NULL};
    return decl;
}

Declarator* appendDeclarator(Declarator* list, Declarator* item) {
    Declarator* tail = list;
    while (tail->next != NULL) tail = tail->next;
    tail->next = item;
    return list;
}

void installDeclarators(DeclType* type, Declarator* list, bool global) {
    while (list != NULL) {
        Declarator* next = list->next;
        if (global) {
            struct Gsymbol* symbol = GInstall(list->name, type->table,
                                               list->pointerLevel);
            if (list->kind == DECL_ARRAY) {
                setArrayLength(symbol, list->arrayLength);
            }
            if (list->kind == DECL_FUNCTION) {
                symbol->paramList = list->parameters;
                symbol->flabel = generateFunctionLabel();
            }
        } else {
            LInstall(list->name, type->table, 0, 0);
        }
        free(list->name);
        free(list);
        list = next;
    }
    free(type);
}
