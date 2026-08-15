#include "translate.h"

FILE* outputFile;

void translate(const char* sourceFile, const char* targetFile) {
    if(sourceFile == NULL || targetFile == NULL) {
        fprintf(stderr, "Error: Source or target file is NULL\n");
        return;
    }
    printf("Translating labels.\n");
    detectin = fopen(sourceFile, "r");
    if (detectin == NULL) {
        fprintf(stderr, "Error: Could not open %s for reading\n", sourceFile);
        return;
    }
    detectlex();
    fclose(detectin);
    translatein = fopen(sourceFile, "r");
    if (translatein == NULL) {
        fprintf(stderr, "Error: Could not open %s for reading\n", sourceFile);
        return;
    }
    outputFile = fopen(targetFile, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error: Could not open %s for writing\n", targetFile);
        return;
    }
    translatelex();
    fclose(translatein);
    fclose(outputFile);
    return;
}