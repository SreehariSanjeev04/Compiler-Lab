#include "memfunclist.h"
#include "logger.h"
#include "flabel.h"

#include <string.h>

struct MemberFuncList *memFuncListHead = NULL;
struct MemberFuncList *memFuncListTail = NULL;
static int nextFuncPosition = 0;

struct MemberFuncList *MemFuncAppend(char* name, struct TypeTable* type) {
    if (name == NULL || name[0] == '\0') {
        LOG_ERROR("Member name cannot be empty.\n");
        exit(1);
    }
    if (type == NULL) {
        LOG_ERROR("Member function type cannot be NULL.\n");
        exit(1);
    }

    struct MemberFuncList *func = malloc(sizeof(*func));
    if (func == NULL) {
        LOG_ERROR("Memory allocation failed for member function.\n");
        return NULL;
    }
    func->name = strdup(name);
    if (func->name == NULL) {
        LOG_ERROR("Memory allocation failed for member function name.\n");
        free(func);
        return NULL;
    }
    func->type = type;
    func->paramList = NULL;
    func->funcPosition = nextFuncPosition++;
    func->Flabel = generateFunctionLabel();
    func->next = NULL;

    if (memFuncListHead == NULL) {
        memFuncListHead = func;
    } else {
        memFuncListTail->next = func;
    }
    memFuncListTail = func;
    return func;
}

struct MemberFuncList *MemFuncGetHead() {
    return memFuncListHead;
}

struct MemberFuncList *MemFuncLookup(char *name) {
    if (name == NULL) {
        return NULL;
    }
    for (struct MemberFuncList *func = memFuncListHead; func != NULL; func = func->next) {
        if (strcmp(func->name, name) == 0) {
            return func;
        }
    }
    return NULL;
}

void MemberFuncInstallParam(struct MemberFuncList* funcPtr, struct ParamList* paramList) {
    if (funcPtr == NULL) {
        LOG_ERROR("Member function cannot be NULL.\n");
        exit(1);
    }
    funcPtr->paramList = paramList;
}

void MemberFuncReset() {
    memFuncListHead = memFuncListTail = NULL;
    nextFuncPosition = 0;
}

void MemberFuncDestroy(struct MemberFuncList* list) {
    if (list == memFuncListHead) {
        MemberFuncReset();
    }
    while (list != NULL) {
        struct MemberFuncList *next = list->next;
        struct ParamList *param = list->paramList;
        while (param != NULL) {
            struct ParamList *nextParam = param->next;
            free(param->name);
            free(param);
            param = nextParam;
        }
        free(list->name);
        free(list);
        list = next;
    }
}
