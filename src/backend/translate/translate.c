#include "translate.h"

int translate(char* sourceFile, char* targetFile) {
    if(sourceFile == NULL || targetFile == NULL) {
        fprintf(stderr, "Error: Source or target file is NULL\n");
        return 1;
    }
    printf("Translating labels.\n");
    FILE* _sourceFile = fopen(sourceFile, "r");
    detectin = _sourceFile;
    if (detectin == NULL) {
        fprintf(stderr, "Error: Could not open output_temp.xsm for reading\n");
        return 1;
    }
    detectlex();
    fclose(detectin);
    translatein = fopen(sourceFile, "r");
    if (translatein == NULL) {
        fprintf(stderr, "Error: Could not open output_temp.xsm for reading\n");
        return 1;
    }
    FILE* _targetFile = fopen(targetFile, "w");
    targetFile = _targetFile;
    translatelex();
    fclose(translatein);
    fclose(targetFile);
    return 0;
}