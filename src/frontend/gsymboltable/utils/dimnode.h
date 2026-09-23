#ifndef DIMNODE_H
#define DIMNODE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct DimNode {
    int size;
    struct DimNode *next;
} DimNode;

extern struct DimNode* headDimNode;
extern struct DimNode* tailDimNode;

/**
 * Creates a dimension node.
 * @param size the dimension size to store
 * @return the new node
 */
struct DimNode* DimNodeCreateNode(int size);
/**
 * @return the head of the dim-node list
 */
struct DimNode* DimNodeGetHead();
/**
 * Resets the dim-node list (head and tail to NULL).
 */
void DimNodeReset();
/**
 * Destroys the dim-node list, freeing all allocated memory.
 */
void DimNodeDestroy(struct DimNode* head);
/**
 * Appends a dimension node with the given size to the end of the list.
 */
void DimNodeAppendNode(int size);

#endif