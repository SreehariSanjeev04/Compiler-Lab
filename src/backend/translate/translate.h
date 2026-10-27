#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdio.h>

extern FILE* detectin;
extern FILE* translatein;
extern FILE* targetFile;
extern int detectlex(void);
extern int translatelex(void);

void translate(const char *sourceFile, const char *targetFile);

#endif