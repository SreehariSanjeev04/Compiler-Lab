#ifndef LABEL_ADDRESS_TABLE_H
#define LABEL_ADDRESS_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LABELS 100

typedef struct LabelTableEntry {
    char* label;
    int address;
} LabelTableEntry;

extern LabelTableEntry labelTable[MAX_LABELS];
extern int labelTableSize;

int getAddress(const char* label);
void storeAddress(const char* label, int address);

#endif