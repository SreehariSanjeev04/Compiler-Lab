#include <typetable.h>
#include <fieldlist.h>

struct TypeTable* head = NULL;

/**
 * Initializes the TypeTable with predefined types: int, bool, str, and void.
 */
void TypeTableCreate() {
    char* types[] = {"int", "bool", "str", "void"};
    int sizes[] = {1,1,1,0};
    
    for(int i = 0; i < 4; i++) {
        if(TInstall(types[i], sizes[i], NULL) == NULL) {
            fprintf(stderr, "Failed to install type '%s'\n", types[i]);
        }
    }
}

/**
 * Destroys the TypeTable and frees all allocated memory.
 */
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

/**
 * Installs a new type in the TypeTable. Returns a pointer to the newly created TypeTable entry, or NULL if the type already exists or memory allocation fails.
 */
struct TypeTable* TInstall(char* name, int size, struct FieldList* fields) {
    struct TypeTable* newType = (struct TypeTable*)malloc(sizeof(struct TypeTable));
    if(!newType) {
        fprintf(stderr, "Memory allocation failed for TypeTable\n");
        return NULL;
    }
    if(TLookup(name) != NULL) {
        fprintf(stderr, "Type '%s' already exists in the TypeTable\n", name);
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

/**
 * Looks up a type in the TypeTable by name. Returns a pointer to the TypeTable entry if found, or NULL if not found.
 */
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

/**
 * Looks up a field in a given type by name. Returns a pointer to the FieldList entry if found, or NULL if not found.
 */
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

/**
 * Returns a pointer to the head of the TypeTable.
 */
struct TypeTable* GetTypeTableHead() {
    return head;
}