#include "logger.h"
#include <paramlist.h> 
#include <constants.h>
#include <string.h> 
#include <exprtree.h>

struct ParamList* headParamList = NULL;
struct ParamList* tailParamList = NULL;

struct ParamList* ParamListGetHead() {
    return headParamList;
}

void ParamListReset() {
    headParamList = NULL;
    tailParamList = NULL;
}

struct ParamList* ParamListGetParam(char *name) {
    struct ParamList* current = headParamList;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int ParamListGetArgCount(struct ParamList *params) {
    int count = 0;
    struct ParamList *param = params;
    while (param) {
        count++;
        param = param->next;
    }
    return count;
}

bool ParamListCheckArgs(struct ParamList* params, struct ParamList* args) {
    if (ParamListGetArgCount(params) != ParamListGetArgCount(args)) return false;
    
    struct ParamList* curr1 = params;
    struct ParamList* curr2 = args;
    while(curr1 != NULL && curr2 != NULL) {
        if (curr1->type != curr2->type || curr1->pointerLevel != curr2->pointerLevel || curr1->cPtr != curr2->cPtr) return false;
        curr1 = curr1->next;
        curr2 = curr2->next;
    }

    return true;
}

struct ParamList* ParamListConvertFromTnode(struct tnode* args) {
    if (args == NULL) return NULL;
    
    struct ParamList* head = NULL;

    struct tnode* curr = args;
    while (curr->nodetype == NODE_TYPE_ARG) {
        struct ParamList* newNode = (struct ParamList*)malloc(sizeof(ParamList));
        struct tnode* node = curr->right;
        newNode->cPtr = node->cPtr;
        newNode->name = node->varname;
        newNode->type = node->typeEntry;
        newNode->pointerLevel = node->pointerLevel;
        newNode->next = NULL;

        if (head == NULL) head = newNode;
        else {
            newNode->next = head;
            head = newNode;
        }

        curr = curr->left;
    }

    struct ParamList* newNode = (struct ParamList*)malloc(sizeof(ParamList));
    struct tnode* node = curr;
    newNode->cPtr = node->cPtr;
    newNode->name = node->varname;
    newNode->type = node->typeEntry;
    newNode->pointerLevel = node->pointerLevel;
    newNode->next = NULL;

    if (head == NULL) head = newNode;
    else {
        newNode->next = head;
        head = newNode;
    }
    return head;
}

struct ParamList* ParamListCreateNode(char *name, struct TypeTable* type, int pointerLevel, struct ClassTable* cptr) {
    struct ParamList* newNode = (struct ParamList*)malloc(sizeof(ParamList));
    newNode->name = strdup(name);
    newNode->type = type;
    newNode->pointerLevel = pointerLevel;
    newNode->cPtr = cptr;
    newNode->next = NULL;
    return newNode;
}

void ParamListAppendNode(char *name, struct TypeTable* type, int pointerLevel, struct ClassTable *cptr) {
    if(ParamListGetParam(name) != NULL) {
        LOG_ERROR("Error: Parameter '%s' is already defined in the parameter list.\n", name);
        exit(1);
    }

    if (cptr != NULL && pointerLevel > 0) {
        LOG_ERROR("Pointer to class not allowed as a parameter\n");
        exit(1);
    }
    struct ParamList* newNode = ParamListCreateNode(name, type, pointerLevel, cptr);

    if (headParamList == NULL) {
        headParamList = newNode;
        tailParamList = newNode;
        return;
    }

    tailParamList->next = newNode;
    tailParamList = newNode;
}

void ParamListDestroy() {
    struct ParamList* current = headParamList;
    struct ParamList* nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current->name);
        free(current);
        current = nextNode;
    }
    ParamListReset();
}

bool ParamListCheckIfParamsMatch(struct ParamList* list1, struct ParamList* list2) {
    struct ParamList* current1 = list1;
    struct ParamList* current2 = list2;

    while (current1 != NULL && current2 != NULL) {
        /* classes wouldn't have type, basically NULL */
        if (strcmp(current1->name, current2->name) != 0 || current1->type != current2->type || current1->cPtr != current2->cPtr ||
            current1->pointerLevel != current2->pointerLevel) {
            return false;
        }
        current1 = current1->next;
        current2 = current2->next;
    }
    return (current1 == NULL && current2 == NULL);
}
