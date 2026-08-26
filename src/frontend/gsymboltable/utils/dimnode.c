#include <dimnode.h>

struct DimNode* headDimNode = NULL;
struct DimNode* tailDimNode = NULL;

/**
 * Creates a new DimNode with the specified size and returns a pointer to it.
 * @param size The size of the dimension to be stored in the node.
 * @return A pointer to the newly created DimNode.
 */
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

/**
 * Appends a new DimNode with the specified size to the end of the linked list.
 * @param size The size of the dimension to be stored in the new node.
 * @return void
 */
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

/**
 * Gets the head of the DimNode linked list.
 * @return A pointer to the head of the DimNode linked list.
 */
struct DimNode* DimNodeGetHead() {
    return headDimNode;
}

/**
 * Destroys the DimNode linked list, freeing all allocated memory.
 * @param head The head of the DimNode linked list to be destroyed.
 * @return void
 */
void DimNodeReset() {
    headDimNode = NULL;
    tailDimNode = NULL;
}

/**
 * Destroys the DimNode linked list, freeing all allocated memory.
 * @param head The head of the DimNode linked list to be destroyed.
 * @return void
 */
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