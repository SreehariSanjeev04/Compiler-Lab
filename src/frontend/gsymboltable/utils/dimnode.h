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

struct DimNode* DimNodeCreateNode(int size);
struct DimNode* DimNodeGetHead();
void DimNodeReset();
void DimNodeDestroy(struct DimNode* head);
void DimNodeAppendNode(int size);

#endif