#include <dimnode.h>

struct DimNode* headDimNode = NULL;

struct DimNode* createDimNode(int size) {
    struct DimNode* newNode = (struct DimNode*)malloc(sizeof(DimNode));
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

struct DimNode* getDimNodeHead() {
    return headDimNode;
}

void setDimNodeHead(struct DimNode* head) {
    headDimNode = head;
}

void clearDimNodeList(struct DimNode* head) {
    struct DimNode* current = head;
    struct DimNode* nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }
    setDimNodeHead(NULL);
}