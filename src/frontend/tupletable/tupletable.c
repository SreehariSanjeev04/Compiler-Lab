#include "tupletable.h"
#include "tuplefieldlist.h"

struct TupleTable *TupleTableHead = NULL;
struct TupleTable *TupleTableTail = NULL;

struct TupleTable *TupleTableGetHead()
{
    return TupleTableHead;
}

void TupleTableAppend(char *name, struct TupleFieldList *fields)
{
    if (TupleTableLookup(name) != NULL)
    {
        fprintf(stderr, "Error: Tuple type '%s' already defined\n", name);
        exit(1);
    }

    TupleTable *node = (TupleTable *)malloc(sizeof(TupleTable));
    if (node == NULL)
    {
        fprintf(stderr, "[ERROR] TupleTable node allocation failed\n");
        exit(1);
    }

    node->name = strdup(name);
    node->next = NULL;
    node->fields = NULL;

    struct TupleFieldList *src = fields;
    struct TupleFieldList *dstTail = NULL;
    while (src != NULL)
    {
        TupleFieldList *copy = (TupleFieldList *)malloc(sizeof(TupleFieldList));
        if (copy == NULL)
        {
            fprintf(stderr, "[ERROR] TupleFieldList copy allocation failed\n");
            exit(1);
        }
        copy->name = strdup(src->name);
        copy->fieldIndex = src->fieldIndex;
        copy->type = src->type;
        copy->size = src->size;
        copy->offset = src->offset;
        copy->next = NULL;
        if (node->fields == NULL)
        {
            node->fields = copy;
            dstTail = copy;
        }
        else
        {
            dstTail->next = copy;
            dstTail = copy;
        }
        src = src->next;
    }

    node->size = TupleTableCalculateSize(node->fields);
    if (TupleTableHead == NULL)
    {
        TupleTableHead = node;
        TupleTableTail = node;
    }
    else
    {
        TupleTableTail->next = node;
        TupleTableTail = node;
    }
}

static int TupleTableCalculateSize(struct TupleFieldList *fields)
{
    struct TupleFieldList *head = fields;
    int size = 0;
    while (head != NULL)
    {
        size += head->size;
        head = head->next;
    }
    return size;
}

void TupleTableDestroy()
{
    TupleTable *current = TupleTableHead;
    while (current != NULL)
    {
        TupleTable *temp = current;
        current = current->next;
        free(temp);
    }
    TupleTableHead = NULL;
    TupleTableTail = NULL;
}

void TupleTableReset()
{
    TupleTableHead = NULL;
    TupleTableTail = NULL;
}

struct TupleTable *TupleTableLookup(char *name)
{
    TupleTable *current = TupleTableHead;
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct TupleFieldList *TupleFieldListLookup(
    char *tuplename,
    char *fieldname)
{
    TupleTable* tupleNode = TupleTableLookup(tuplename);
    if (tupleNode == NULL) {
        return NULL;
    }

    TupleFieldList* current = tupleNode->fields;
    while(current != NULL) {
        if(strcmp(current->name, fieldname) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
