#ifndef CLASSTABLE_H
#define CLASSTABLE_H

struct ParamList;

#include <fieldlist.h>
#include <memfunclist.h>

typedef struct ClassTable {
    char *name;
    int vftAddress;
    struct FieldList *memberField;
    struct ClassTable *parentPtr;
    struct MemberFuncList *vfuncptr;
    int classIdx;
    int fieldCount;
    int methodCount;
    struct ClassTable* next;
} ClassTable;

struct ClassTable *CInstall(char *name, char* parentClassName);
struct ClassTable* CLookup(char *name);
void CFieldInstall(struct ClassTable *cptr, struct FieldList* fields);
void CMemInstall(struct ClassTable *cptr, struct MemberFuncList* methods);
struct MemberFuncList* CMemLookup(struct ClassTable* cptr, char *name, struct ParamList* paramlist);
struct FieldList* CFieldLookup(struct ClassTable* ctype, char* name);
int CGetClassCount(void);
struct ClassTable *ClassTableGetHead(void);
#endif
