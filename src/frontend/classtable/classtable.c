#include "classtable.h"
#include "logger.h"

#include <string.h>

static int counter = 0;

struct ClassTable *classTableListHead = NULL;
struct ClassTable *classTableListTail = NULL;

struct ClassTable *CInstall(char *name, char* parentClassName) {
    if (name == NULL || name[0] == '\0') {
        LOG_ERROR("Class name should not be empty.\n");
        exit(1);
    }

    if (CLookup(name) != NULL) {
        LOG_ERROR("Class '%s' is already defined.\n", name);
        exit(1);
    }

    struct ClassTable *parent = NULL;
    if (parentClassName != NULL && parentClassName[0] != '\0') {
        parent = CLookup(parentClassName);
        if (parent == NULL) {
            LOG_ERROR("Parent class '%s' is not defined.\n", parentClassName);
            exit(1);
        }
    }

    struct ClassTable* classPtr = malloc(sizeof(*classPtr));
    if (classPtr == NULL) {
        LOG_ERROR("Memory allocation failed for class.\n");
        return NULL;
    }
    classPtr->name = strdup(name);
    if (classPtr->name == NULL) {
        LOG_ERROR("Memory allocation failed for class name.\n");
        free(classPtr);
        return NULL;
    }
    classPtr->parentPtr = parent;
    classPtr->classIdx = counter++;
    classPtr->vfuncptr = NULL; 
    classPtr->memberField = NULL; 
    classPtr->fieldCount = 0;
    classPtr->methodCount = 0;
    classPtr->next = NULL;
    
    if (classTableListHead == NULL) {
        classTableListHead = classTableListTail = classPtr;
    } else {
        classTableListTail->next = classPtr;
        classTableListTail = classPtr;
    }
    return classPtr;
}
struct ClassTable* CLookup(char *name) {
    if (name == NULL || name[0] == '\0') {
        return NULL;
    }
    struct ClassTable *current = classTableListHead;
    while(current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL; 
}
void CFieldInstall(struct ClassTable *cptr, struct FieldList* fields) {
    if (cptr == NULL) {
        LOG_ERROR("The class pointer should not be NULL.\n");
        exit(1);
    }
    cptr->memberField = fields;
    cptr->fieldCount = 0;
    for (struct FieldList *field = fields; field != NULL; field = field->next) {
        cptr->fieldCount++;
        
        if (cptr->fieldCount > 8) {
            LOG_ERROR("Maximum 8 fields allowed in each class\n");
            exit(1);
        }
    }
}
void CMemInstall(struct ClassTable *cptr, struct MemberFuncList* methods) {
    if (cptr == NULL) {
        LOG_ERROR("The class pointer should not be NULL.\n");
        exit(1);
    }
    cptr->vfuncptr = methods;
    cptr->methodCount = 0;
    for (struct MemberFuncList *method = methods; method != NULL; method = method->next) {
        cptr->methodCount++;

        if (cptr->methodCount > 8) {
            LOG_ERROR("Maximum 8 methods allowed in each class\n");
            exit(1);
        }
    }
}
struct MemberFuncList* CMemLookup(struct ClassTable* cptr, char *name) {
    if (cptr == NULL) {
        LOG_ERROR("The class pointer should not be NULL.\n");
        exit(1);
    }
    if (name == NULL || name[0] == '\0') {
        LOG_ERROR("The method name should not be empty.\n");
        exit(1);
    }
    for (struct ClassTable *cls = cptr; cls != NULL; cls = cls->parentPtr) {
        for (struct MemberFuncList *current = cls->vfuncptr; current != NULL; current = current->next) {
            if (strcmp(current->name, name) == 0) {
                return current;
            }
        }
    }
    return NULL;
}
struct FieldList* CFieldLookup(struct ClassTable* cptr, char* name) {
    if (cptr == NULL) {
        LOG_ERROR("The class pointer should not be NULL.\n");
        exit(1);
    }
    if (name == NULL || name[0] == '\0') {
        LOG_ERROR("The field name should not be empty.\n");
        exit(1);
    }
    for (struct ClassTable *cls = cptr; cls != NULL; cls = cls->parentPtr) {
        for (struct FieldList *current = cls->memberField; current != NULL; current = current->next) {
            if (strcmp(current->name, name) == 0) {
                return current;
            }
        }
    }
    return NULL;
}
