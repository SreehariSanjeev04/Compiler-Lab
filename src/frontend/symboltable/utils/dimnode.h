#ifndef DIMNODE_H
#define DIMNODE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct DimNode {
    int size;
    struct DimNode *next;
} DimNode;

struct DimNode* createDimNode(int size);
struct DimNode* appendDimNode(struct DimNode* head, int size);

#endif