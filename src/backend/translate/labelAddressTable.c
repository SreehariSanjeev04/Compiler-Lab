#include "labelAddressTable.h"

LabelTableEntry labelTable[100]; 
int labelTableSize = 0;
int getAddress(char* label) {
    for (int i = 0; i < labelTableSize; i++) {
        if (strcmp(labelTable[i].label, label) == 0) {
            return labelTable[i].address;
        }
    }
    fprintf(stderr, "Error: Label %s not found\n", label);
    exit(1);
}

void storeAddress(char* label, int address) {
    if (labelTableSize >= 100) {
        fprintf(stderr, "Error: Label table is full\n");
        exit(1);
    }
    labelTable[labelTableSize].label = (char*)malloc(strlen(label) + 1);
    strcpy(labelTable[labelTableSize].label, label);
    labelTable[labelTableSize].address = address;
    labelTableSize++;
}