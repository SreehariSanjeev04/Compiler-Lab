#include "logger.h"
#include <paramlist.h> 
#include <constants.h>
#include <string.h> 

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
