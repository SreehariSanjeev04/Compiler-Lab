#include "translate.h"
extern FILE* outputFile;

void translate(char* sourceFile, char* targetFile) {
    if(sourceFile == NULL || targetFile == NULL) {
        fprintf(stderr, "Error: Source or target file is NULL\n");
        return;
    }
    printf("Translating labels.\n");
    FILE* _sourceFile = fopen(sourceFile, "r");
    detectin = _sourceFile;
    if (detectin == NULL) {
        fprintf(stderr, "Error: Could not open output_temp.xsm for reading\n");
        return;
    }
    detectlex();
    fclose(detectin);
    translatein = fopen(sourceFile, "r");
    if (translatein == NULL) {
        fprintf(stderr, "Error: Could not open output_temp.xsm for reading\n");
        return;
    }
    translatein = fopen("output_temp.xsm", "r");
    if (translatein == NULL) {
        fprintf(stderr, "Error: Could not open output_temp.xsm for reading\n");
        return;
    }
    outputFile = fopen(targetFile, "w");
    translatelex();
    fclose(translatein);
    return;
}