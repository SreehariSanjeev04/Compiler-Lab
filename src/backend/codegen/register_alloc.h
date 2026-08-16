#ifndef REGISTER_ALLOC_H
#define REGISTER_ALLOC_H

#include <stdio.h>

int getReg(void);
void freeReg(void);
int generateLabel(void);
void saveRegisters(FILE *targetFile);
void restoreRegisters(FILE *targetFile);

extern int regCount;
extern int labelCount;

#endif // REGISTER_ALLOC_H
