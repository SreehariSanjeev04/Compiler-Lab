#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdio.h>

void addHeader(FILE *targetFile);
void printValue(int reg, FILE *targetFile);
void readValue(int reg, FILE *targetFile);
void exitProgram(FILE *targetFile);

#endif // RUNTIME_H
