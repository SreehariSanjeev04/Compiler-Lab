#ifndef PARAMLIST_H
#define PARAMLIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct ParamList {
    char *name;
    int type;
    struct ParamList *next;
} ParamList;

extern struct ParamList* headParamList;

struct ParamList* createParamListNode(char *name, int type);
struct ParamList* appendParamListNode(struct ParamList* head, char *name, int type);
struct ParamList* getParamListHead();
struct ParamList* setParamListHead(struct ParamList* head);
struct ParamList* getParam(char *name);
void freeParamList(struct ParamList* head);
#endif