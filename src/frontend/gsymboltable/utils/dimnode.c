#include <dimnode.h>

struct DimNode* headDimNode = NULL;
struct DimNode* tailDimNode = NULL;

struct DimNode* DimNodeCreateNode(int size) {
    struct DimNode* newNode = (struct DimNode*)malloc(sizeof(DimNode));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for DimNode\n");
        exit(1);
    }
    newNode->size = size;
    newNode->next = NULL;
    return newNode;
}

void DimNodeAppendNode(int size) {
    struct DimNode* newNode = DimNodeCreateNode(size);
    if (headDimNode == NULL) {
        headDimNode = newNode;
        tailDimNode = newNode;
        return;
    }

    tailDimNode->next = newNode;
    tailDimNode = newNode;
}

struct DimNode* DimNodeGetHead() {
    return headDimNode;
}

void DimNodeReset() {
    headDimNode = NULL;
    tailDimNode = NULL;
}

void DimNodeDestroy(struct DimNode* head) {
    struct DimNode* current = head;
    struct DimNode* nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }
    DimNodeReset();
}