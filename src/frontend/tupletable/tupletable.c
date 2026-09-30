#include "logger.h"
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
        struct TupleTable* temp = TupleTableLookup(name);
        if (!TupleTableCheckIfFieldsMatch(temp->fields, fields)) {
            LOG_ERROR("The tuple is already declared but fields do not match.");
            exit(1);
        }
    }

    TupleTable *node = (TupleTable *)malloc(sizeof(TupleTable));
    if (node == NULL)
    {
        LOG_ERROR("TupleTable node allocation failed\n");
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
            LOG_ERROR("TupleFieldList copy allocation failed\n");
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

bool TupleTableCheckIfFieldsMatch(
    TupleFieldList* a,
    TupleFieldList* b
) {
    TupleFieldList* current1 = a;
    TupleFieldList* current2 = b;

    while(current1 != NULL && current2 != NULL) {
        if (strcmp(current1->name,current2->name) != 0 ||
            current1->type != current2->type) return false;
            
            current1 = current1->next;
            current2 = current2->next;
    }

    if (current1 != NULL) return false;
    if (current2 != NULL) return false;

    return true;
}