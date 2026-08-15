#include "labelAddressTable.h"

LabelTableEntry labelTable[MAX_LABELS]; 
int labelTableSize = 0;
int getAddress(const char* label) {
    for (int i = 0; i < labelTableSize; i++) {
        if (strcmp(labelTable[i].label, label) == 0) {
            return labelTable[i].address;
        }
    }
    fprintf(stderr, "Error: Label %s not found\n", label);
    exit(1);
}

void storeAddress(const char* label, int address) {
    if (labelTableSize >= MAX_LABELS) {
        fprintf(stderr, "Error: Label table is full\n");
        exit(1);
    }
    labelTable[labelTableSize].label = (char*)malloc(strlen(label) + 1);
    strcpy(labelTable[labelTableSize].label, label);
    labelTable[labelTableSize].address = address;
    labelTableSize++;
}