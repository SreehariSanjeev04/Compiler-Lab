#ifndef PARAMLIST_H
#define PARAMLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct ParamList {
    char *name;
    int type;
    struct ParamList *next;
} ParamList;

extern struct ParamList* headParamList;
extern struct ParamList* tailParamList;

struct ParamList* ParamListCreateNode(char *name, int type);
struct ParamList* ParamListGetHead();
struct ParamList* ParamListGetParam(char *name);
bool ParamListCheckIfParamsMatch(struct ParamList* list1, struct ParamList* list2);
void ParamListDestroy();
void ParamListReset();
void ParamListAppendNode(char *name, int type);

#endif