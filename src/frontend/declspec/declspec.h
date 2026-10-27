#ifndef DECLSPEC_H
#define DECLSPEC_H

#include <stdbool.h>

struct TypeTable;
struct ClassTable;
struct ParamList;

typedef struct DeclType {
    int kind;
    struct TypeTable *table;
    struct ClassTable* cptr;
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

DeclType* makeDeclTypeForType(int kind, struct TypeTable* table);
DeclType* makeDeclTypeForClass(int kind, struct ClassTable* cptr);
Declarator* makeDeclarator(char* name, DeclaratorKind kind, int pointerLevel,
                            int arrayLength, struct ParamList* parameters);
Declarator* appendDeclarator(Declarator* list, Declarator* item);
void installDeclarators(DeclType* type, Declarator* list, bool global);

#endif
