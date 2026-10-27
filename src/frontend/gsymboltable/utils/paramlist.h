#ifndef PARAMLIST_H
#define PARAMLIST_H

struct tnode;

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <typetable.h>
#include <classtable.h>

typedef struct ParamList {
    char *name;
    struct TypeTable* type;
    int pointerLevel;
    struct ClassTable *cPtr; // pointer to the class entry
    struct ParamList *next;
} ParamList;

extern struct ParamList* headParamList;
extern struct ParamList* tailParamList;

/**
 * Creates a new parameter node.
 * @return the newly created ParamList node
 */
struct ParamList* ParamListCreateNode(char *name, struct TypeTable* type, int pointerLevel, struct ClassTable *cptr);
/**
 * @return the head of the parameter list
 */
struct ParamList* ParamListGetHead();
/**
 * Gets a parameter by name.
 * @return the ParamList node if found, or NULL if not found
 */
struct ParamList* ParamListGetParam(char *name);
/**
 * Checks whether two parameter lists match in name, type, and pointer level.
 * @return true if they match, false otherwise
 */
bool ParamListCheckIfParamsMatch(struct ParamList* list1, struct ParamList* list2);
/**
 * Destroys the parameter list, freeing all allocated memory.
 */
void ParamListDestroy();
/**
 * Resets the parameter list (head and tail to NULL).
 */
void ParamListReset();
/**
 * Appends a new parameter to the end of the list.
 */
void ParamListAppendNode(char *name, struct TypeTable* type, int pointerLevel, struct ClassTable* cptr);

int ParamListGetArgCount(struct ParamList *params);

bool ParamListCheckArgs(struct ParamList* params, struct ParamList* args);

struct ParamList* ParamListConvertFromTnode(struct tnode* args);

#endif
