#include "logger.h"
#include <typetable.h>
#include <fieldlist.h>

struct TypeTable* head = NULL;

void TypeTableCreate() {
    char* types[] = {"int", "bool", "str", "void"};
    int sizes[] = {1,1,1,0};
    
    for(int i = 0; i < 4; i++) {
        if(TInstall(types[i], sizes[i], NULL) == NULL) {
            LOG_ERROR("Failed to install type '%s'\n", types[i]);
        }
    }
}

void TypeTableDestroy() {
    struct TypeTable* current = head;
    while(current != NULL) {
        struct TypeTable* temp = current;
        current = current->next;
        free(temp->name);
        struct FieldList* fieldCurrent = temp->fields;
        while(fieldCurrent != NULL) {
            struct FieldList* fieldTemp = fieldCurrent;
            fieldCurrent = fieldCurrent->next;
            free(fieldTemp->name);
            free(fieldTemp);
        }
        free(temp);
    }
    head = NULL;
}

struct TypeTable* TInstall(char* name, int size, struct FieldList* fields) {
    struct TypeTable* newType = (struct TypeTable*)malloc(sizeof(struct TypeTable));
    if(!newType) {
        LOG_ERROR("Memory allocation failed for TypeTable\n");
        return NULL;
    }
    if(TLookup(name) != NULL) {
        LOG_ERROR("Type '%s' already exists in the TypeTable\n", name);
        free(newType);
        return NULL;
    }
    newType->name = name;
    newType->size = size;
    newType->fields = fields;
    newType->next = NULL;
    if(head == NULL) {
        head = newType;
    } else {
        struct TypeTable* current = head;
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = newType;
    }
    return newType;
}

struct TypeTable* TLookup(char *name) {
    struct TypeTable* current = head;
    while(current != NULL) {
        if(strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct FieldList* FLookup(TypeTable* type, char* name) {
    if(type == NULL) {
        return NULL;
    }
    struct FieldList* current = type->fields;
    while(current != NULL) {
        if(strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct TypeTable* GetTypeTableHead() {
    return head;
}