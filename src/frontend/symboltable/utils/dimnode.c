#include <dimnode.h>

struct DimNode* createDimNode(int size) {
    
    DimNode *newNode = (DimNode *)malloc(sizeof(DimNode));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for DimNode\n");
        exit(1);
    }
    newNode->size = size;
    newNode->next = NULL;
    return newNode;
}

struct DimNode* appendDimNode(struct DimNode* head, int size) {
    struct DimNode* newNode = createDimNode(size);
    if (head == NULL) return newNode;

    struct DimNode* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
    return head;
}