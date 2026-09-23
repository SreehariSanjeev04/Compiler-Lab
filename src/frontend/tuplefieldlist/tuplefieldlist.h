#ifndef TUPLEFIELDLIST_H
#define TUPLEFIELDLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "constants.h"

/* Only scalar fields are supported to keep size calculation simple. */

typedef struct TupleFieldList {
    char* name; // name of the field
    int fieldIndex; // the position of the field in the field list
    int type; // the type of the field
    int size; // size of the field
    int offset; // word offset of the field within its tuple
    struct TupleFieldList* next; // next pointer
} TupleFieldList;

/**
 * Appends the field to the existing FieldList
 * @param name Name of the field
 * @param type Type of the field
 */
void TupleFieldListAppend(char* name, int type);

/**
 * Resets the FieldList by freeing the allocated memory
 */
void TupleFieldListReset();

/**
 * Returns the head node of the existing FieldList
 * @return The head node
 */
struct TupleFieldList* TupleFieldListGetHead();

#endif