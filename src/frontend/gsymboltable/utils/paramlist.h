#ifndef PARAMLIST_H
#define PARAMLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tupletable.h"

typedef struct ParamList {
    char *name;
    int type;
    int pointerLevel;
    struct TupleTable* tupleEntry; // only when the parameter has a tuple entry
    struct ParamList *next;
} ParamList;

extern struct ParamList* headParamList;
extern struct ParamList* tailParamList;

/**
 * Creates a new parameter node.
 * @return the newly created ParamList node
 */
struct ParamList* ParamListCreateNode(char *name, int type, int pointerLevel);
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
void ParamListAppendNode(char *name, int type, int pointerLevel);

#endif