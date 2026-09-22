#include <paramlist.h> 
#include <string.h> 

struct ParamList* headParamList = NULL;
struct ParamList* tailParamList = NULL;

/**
 * Returns the head of the parameter list.
 * @return A pointer to the head of the parameter list.
 */
struct ParamList* ParamListGetHead() {
    return headParamList;
}

/**
 * Resets the parameter list by setting the head and tail pointers to NULL.
 */
void ParamListReset() {
    headParamList = NULL;
    tailParamList = NULL;
}

/**
 * Gets a parameter from the parameter list by its name.
 * @param name The name of the parameter to look up.
 * @return A pointer to the ParamList node if found, or NULL if not found.
 */
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

/**
 * Creates a new parameter list node with the given name and type.
 * @param name The name of the parameter.
 * @param type The type of the parameter.
 * @return A pointer to the newly created ParamList node.
 */
struct ParamList* ParamListCreateNode(char *name, int type, int pointerLevel) {
    struct ParamList* newNode = (struct ParamList*)malloc(sizeof(ParamList));
    newNode->name = strdup(name);
    newNode->type = type;
    newNode->pointerLevel = pointerLevel;
    newNode->next = NULL;
    return newNode;
}

/**
 * Appends a new parameter to the end of the parameter list.
 * @param name The name of the parameter.
 * @param type The type of the parameter.
 * @return A pointer to the newly created ParamList node.
 */
void ParamListAppendNode(char *name, int type, int pointerLevel) {
    if(ParamListGetParam(name) != NULL) {
        fprintf(stderr, "Error: Parameter '%s' is already defined in the parameter list.\n", name);
        exit(1);
    }
    struct ParamList* newNode = ParamListCreateNode(name, type, pointerLevel);

    if (headParamList == NULL) {
        headParamList = newNode;
        tailParamList = newNode;
        return;
    }

    tailParamList->next = newNode;
    tailParamList = newNode;
}

/**
 * Destroys the parameter list, freeing all allocated memory.
 */
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

/**
 * Checks if two parameter lists match in terms of names and types.
 * @param list1 The first parameter list.
 * @param list2 The second parameter list.
 * @return true if the parameter lists match, false otherwise.
 */
bool ParamListCheckIfParamsMatch(struct ParamList* list1, struct ParamList* list2) {
    struct ParamList* current1 = list1;
    struct ParamList* current2 = list2;

    while (current1 != NULL && current2 != NULL) {
        if (strcmp(current1->name, current2->name) != 0 || current1->type != current2->type || current1->pointerLevel != current2->pointerLevel) {
            return false;
        }
        current1 = current1->next;
        current2 = current2->next;
    }
    return (current1 == NULL && current2 == NULL);
}