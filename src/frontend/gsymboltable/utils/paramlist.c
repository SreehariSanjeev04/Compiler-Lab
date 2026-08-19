#include <paramlist.h> 

struct ParamList* headParamList = NULL;

struct ParamList* getParamListHead() {
    return headParamList;
}

struct ParamList* setParamListHead(struct ParamList* head) {
    headParamList = head;
    return headParamList;
}

struct ParamList* getParam(char *name) {
    struct ParamList* current = headParamList;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct ParamList* createParamListNode(char *name, int type) {
    struct ParamList* newNode = (struct ParamList*)malloc(sizeof(ParamList));
    newNode->name = name;
    newNode->type = type;
    newNode->next = NULL;
    return newNode;
}

struct ParamList* appendParamListNode(struct ParamList* head, char *name, int type) {
    if(getParam(name) != NULL) {
        fprintf(stderr, "Error: Parameter '%s' is already defined in the parameter list.\n", name);
        exit(1);
    }
    struct ParamList* newNode = createParamListNode(name, type);

    if (head == NULL) return newNode;

    struct ParamList* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
    return head;
}

void freeParamList(struct ParamList* head) {
    struct ParamList* current = head;
    struct ParamList* nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }
    setParamListHead(NULL);
}