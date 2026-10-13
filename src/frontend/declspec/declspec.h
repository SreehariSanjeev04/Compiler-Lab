#ifndef DECLSPEC_H
#define DECLSPEC_H

#include <stdbool.h>

struct TypeTable;
struct ParamList;

typedef struct DeclType {
    int kind;
    struct TypeTable *table;
} DeclType;

typedef enum DeclaratorKind {
    DECL_VARIABLE,
    DECL_POINTER,
    DECL_ARRAY,
    DECL_FUNCTION
} DeclaratorKind;

typedef struct Declarator {
    char *name;
    DeclaratorKind kind;
    int pointerLevel;
    int arrayLength;
    struct ParamList *parameters;
    struct Declarator *next;
} Declarator;

DeclType* makeDeclType(int kind, struct TypeTable* table);
Declarator* makeDeclarator(char* name, DeclaratorKind kind, int pointerLevel,
                            int arrayLength, struct ParamList* parameters);
Declarator* appendDeclarator(Declarator* list, Declarator* item);
void installDeclarators(DeclType* type, Declarator* list, bool global);

#endif
