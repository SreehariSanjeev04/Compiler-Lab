#ifndef MEMFUNCLIST_H
#define MEMFUNCLIST_H
#include <paramlist.h>
#include <typetable.h>

typedef struct MemberFuncList {
    char *name;
    struct TypeTable* type;
    struct ParamList* paramList;
    int funcPosition;
    int Flabel;
    struct MemberFuncList *next;
} MemberFuncList;

struct MemberFuncList *MemFuncAppend(char* name, struct TypeTable* type);
struct MemberFuncList *MemFuncLookup(char *name);
struct MemberFuncList *MemFuncGetHead();
void MemberFuncInstallParam(struct MemberFuncList* funcPtr, struct ParamList* paramList);
void MemberFuncReset();
void MemberFuncDestroy(struct MemberFuncList* list);
#endif
