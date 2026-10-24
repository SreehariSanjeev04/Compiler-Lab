#include "classtable.h"
#include "logger.h"
#include "constants.h"

#include <string.h>

static int counter = 0;

struct ClassTable *classTableListHead = NULL;
struct ClassTable *classTableListTail = NULL;

int CGetClassCount(void) {
    int count = 0;
    struct ClassTable *current = classTableListHead;
    while(current != NULL) {
        count++;
        current = current->next;
    }
    return count;
} 

struct ClassTable *ClassTableGetHead(void) {
    return classTableListHead;
}
static struct MemberFuncList *duplicateMethods(
    const struct MemberFuncList *methods
) {
    struct MemberFuncList *head = NULL;
    struct MemberFuncList *tail = NULL;

    for (const struct MemberFuncList *curr = methods;
         curr != NULL;
         curr = curr->next) {

        struct MemberFuncList *node = calloc(1, sizeof(*node));

        if (node == NULL) {
            LOG_ERROR("Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        node->name = strdup(curr->name);

        if (node->name == NULL) {
            LOG_ERROR("strdup failed\n");
            free(node);
            exit(EXIT_FAILURE);
        }

        node->Flabel = curr->Flabel;
        node->funcPosition = curr->funcPosition;
        node->paramList = curr->paramList; 
        node->type = curr->type;

        if (head == NULL) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    return head;
}

static struct FieldList *duplicateFields(struct FieldList *fields) {
    if (fields == NULL)
        return NULL;

    struct FieldList *head = NULL;
    struct FieldList *tail = NULL;

    for (struct FieldList *curr = fields;
         curr != NULL;
         curr = curr->next) {

        struct FieldList *node = malloc(sizeof(*node));

        if (node == NULL) {
            LOG_ERROR("Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        node->ctype = curr->ctype;
        node->fieldIndex = curr->fieldIndex;
        node->name = strdup(curr->name);
        node->size = curr->size;
        node->type = curr->type;
        node->next = NULL; 

        if (node->name == NULL) {
            LOG_ERROR("strdup failed\n");
            free(node);
            exit(EXIT_FAILURE);
        }

        if (head == NULL) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    return head;
}

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
    classPtr->vftAddress = DEFAULT_BINDING_ADDRESS + classPtr->classIdx * 8;
    classPtr->vfuncptr = parent != NULL ? duplicateMethods(parent->vfuncptr): NULL; 
    classPtr->memberField = parent != NULL ? duplicateFields(parent->memberField) : NULL;
    
    // if the parent class is present, child inherits them
    classPtr->fieldCount = parent != NULL ? parent->fieldCount : 0;
    classPtr->methodCount = parent != NULL ? parent->methodCount : 0;
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

/* Todo: fix issue*/

static bool checkFieldsAgainstParentClass(struct FieldList *fieldList, struct ClassTable *parentPtr) {
    if (parentPtr == NULL) {
        LOG_ERROR("Parent pointer should not be NULL\n");
        exit(1);
    }
    struct FieldList *current = fieldList;
    while(current) {
        struct FieldList *parentFieldEntry = CFieldLookup(parentPtr, current->name);
        if (parentFieldEntry != NULL) {
            // the field exists in parent, does not matter the type
            return false;
        }
        current = current->next;
    }
    return true;
}

static int countParams(struct MemberFuncList* method1) {
    if (method1 == NULL) {
        LOG_ERROR("Method pointer should not be NULL to count parameters\n");
        exit(1);
    }
    int count = 0;
    struct ParamList* curr = method1->paramList;
    while(curr != NULL) {
        count++;
        curr = curr->next;
    }
    return count;
}

static bool checkMethodSignature(struct MemberFuncList *method1, struct MemberFuncList* method2) {
    if (method1 == NULL || method2 == NULL) {
        LOG_ERROR("Methods should not be NULL to verify the signature\n");
        exit(1);
    }

    if (method1->type != method2->type) return false; // method overloading
    
    if (countParams(method1) != countParams(method2)) return false;
    struct ParamList* curr1 = method1->paramList;
    struct ParamList* curr2 = method2->paramList;

    // not checking pointer levels as of now
    while (curr1 != NULL && curr2 != NULL) {
        if (curr1->type != curr2->type || curr1->cPtr != curr2->cPtr || curr1->pointerLevel != curr2->pointerLevel) return false;
        curr1 = curr1->next;
        curr2 = curr2->next;
    }

    return true;
}

static struct FieldList* appendFields(struct FieldList* old, struct FieldList* new) {
    if (new == NULL) {
        LOG_INFO("No fields to append\n");
        return old;
    }
    if (old == NULL) return new;
    
    struct FieldList* current = old;
    while(current->next != NULL) current = current->next;
    current->next = new;

    // fix the messed up fieldIndex

    current = old;
    int offset = 0;
    while(current != NULL) {
        current->fieldIndex = offset;
        offset = current->ctype != NULL ? offset + 2 : offset + 1;
        current = current->next;
    }

    return old;
}

static struct MemberFuncList* appendMethods(struct MemberFuncList* old, struct MemberFuncList *new) {
    if (new == NULL) {
        LOG_INFO("No methods to append\n");
        return old;
    }
    if (old == NULL) return new;
    
    struct MemberFuncList* current = old;
    while(current->next != NULL) current = current->next;
    current->next = new;

    current = old;
    int offset = 0;

    // fix the function position
    while(current != NULL) {
        current->funcPosition = offset++;
        current = current->next;
    }

    return old;
}

void CFieldInstall(struct ClassTable *cptr, struct FieldList* fields) {
    if (cptr == NULL) {
        LOG_ERROR("The class pointer should not be NULL.\n");
        exit(1);
    }
    if (cptr->parentPtr != NULL) {
        // inheritance
        if (!checkFieldsAgainstParentClass(fields, cptr->parentPtr)) {
            LOG_ERROR("Parent fields cannot be declared\n");
            exit(1);
        }
    }
    cptr->memberField = appendFields(cptr->memberField, fields);
    for (struct FieldList *field = fields; field != NULL; field = field->next) {
        if (field->ctype != NULL) cptr->fieldCount+=2; // to store the vft as well
        else cptr->fieldCount++;
        
        if (cptr->fieldCount > 8) {
            LOG_ERROR("Maximum 8 fields allowed in each class\n");
            exit(1);
        }
    }
}

static struct MemberFuncList* deleteMethod(struct MemberFuncList* list, const char* name) {
    if (list == NULL) return NULL;
    if (name[0] == '\0') return list;

    struct MemberFuncList *curr = list;
    struct MemberFuncList* head = list;

    if (strcmp(head->name, name) == 0) return head->next;
    while(curr->next != NULL && strcmp(curr->next->name, name) != 0) curr = curr->next;

    struct MemberFuncList* deleteNode = curr->next;
    curr->next = curr->next->next;

    free(deleteNode->name);
    free(deleteNode);

    return head;
}

/* For methods, if the overriden, erase parent's method from existence */

void CMemInstall(struct ClassTable *cptr, struct MemberFuncList* methods) {
    if (cptr == NULL) {
        LOG_ERROR("The class pointer should not be NULL.\n");
        exit(1);
    }

    if (cptr->parentPtr != NULL) {
        struct MemberFuncList *curr = methods;
        while(curr != NULL) {
            struct MemberFuncList *next = curr->next;
            struct MemberFuncList *parentMethodEntry = CMemLookup(cptr->parentPtr, curr->name);

            if (parentMethodEntry != NULL) {
                if (!checkMethodSignature(curr, parentMethodEntry)) {
                    LOG_ERROR("Method overloading not permitted\n");
                    exit(1);
                }

                struct MemberFuncList *curr2 = cptr->vfuncptr;
                while(curr2 && strcmp(curr2->name, curr->name) != 0) curr2 = curr2->next;
                
                if(curr2 == NULL) {
                    LOG_ERROR("Could not find the method in method list of class %s\n", cptr->name);
                    exit(1);
                }

                /* The methods have to be inherited during CInstall() */

                curr2->Flabel = curr->Flabel; 
                curr2->paramList = curr->paramList;

                methods = deleteMethod(methods, curr->name);
            }
            curr = next;
        }
    }
    cptr->vfuncptr = appendMethods(cptr->vfuncptr, methods);
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
