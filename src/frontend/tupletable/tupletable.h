#ifndef TUPLETABLE_H
#define TUPLETABLE_H

#include <stdlib.h>
#include <stdbool.h>

#include "tuplefieldlist.h"
// Very similar to the TypeTable, but created a dedicated table for the sake of simplicity

typedef struct TupleTable
{
    char *name;
    int size; // no struct padding and all
    struct TupleFieldList *fields;
    struct TupleTable *next;
} TupleTable;

/**
 * Returns the head node of the current TupleTable list
 * @returns head node
 */
struct TupleTable *TupleTableGetHead();

/**
 * Appends the node to the current TupleTable list
 * @param name The name of the tuple
 * @param fields The pointer ot the fields of the tuple
 */
void TupleTableAppend(char *name, struct TupleFieldList *fields);

/**
 * Calculates the size of the tuple.
 * This is done by adding the sizes of indivitual field
 * @param fields Pointer to the fieldlist
 * @returns The summation of the sizes of each field
 */
static int TupleTableCalculateSize(struct TupleFieldList *fields);

/**
 * Returns the tuple whose name matches with the provided one
 * @param name The name of the tuple
 * @returns Returns the corresponding tuple or NULL
 */
struct TupleTable *TupleTableLookup(char *name);

/**
 * Resets the TupleTable list by freeing the memory allocated for the nodes
 */
void TupleTableReset();

/**
 * Returns the field list node corresponding to the provided name
 * @param tuplename The name of the tuple
 * @param fieldname The name of the field
 * @returns The field list node with the provided tuple name and field name
 */
struct TupleFieldList *TupleFieldListLookup(
    char *tuplename,
    char *fieldname);

bool TupleTableCheckIfFieldsMatch(
    TupleFieldList* a,
    TupleFieldList* b
);
#endif