#ifndef LABEL_ADDRESS_TABLE_H
#define LABEL_ADDRESS_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LabelTableEntry {
    char* label;
    int address;
} LabelTableEntry;

LabelTableEntry labelTable[100]; // Assuming a maximum of 100 labels
int labelTableSize = 0;

int getAddress(char* label);
void storeAddress(char* label, int address);

#endif LABEL_ADDRESS_TABLE_H