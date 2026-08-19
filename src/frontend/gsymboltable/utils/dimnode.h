#ifndef DIMNODE_H
#define DIMNODE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct DimNode {
    int size;
    struct DimNode *next;
} DimNode;

extern struct DimNode* headDimNode;

struct DimNode* createDimNode(int size);
struct DimNode* appendDimNode(struct DimNode* head, int size);
struct DimNode* getDimNodeHead();
void setDimNodeHead(struct DimNode* head);
void clearDimNodeList(struct DimNode* head);

#endif