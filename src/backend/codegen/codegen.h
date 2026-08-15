#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "exprtree.h"
#include "constants.h"

int getReg(void);
void freeReg(void);
int codeGen(tnode* root, FILE* targetFile);
int codeGenArrayAddress(tnode* root, FILE* targetFile);
int codeGenAddressOperand(tnode* root, FILE* targetFile);
void addHeader(FILE* targetFile);
void printValue(int reg, FILE* targetFile);
void readValue(int reg, FILE* targetFile);
void saveRegisters(FILE* targetFile);
void restoreRegisters(FILE* targetFile);
void exitProgram(FILE* targetFile);
int returnStaticBindAddress(const char* varname);
void generateCode(tnode* root, FILE* targetFile);
int generateLabel(void);

extern int regCount;
extern int labelCount;

#endif // CODEGEN_H