#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdio.h>

extern FILE* detectin;
extern FILE* translatein;
extern FILE* targetFile;
extern int detectlex();
extern int translatelex();

void translate(char *input,
               char *output);

#endif