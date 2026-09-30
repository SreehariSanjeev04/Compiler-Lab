#include "logger.h"
#include "translate.h"

FILE* outputFile;

void translate(const char* sourceFile, const char* targetFile) {
    if(sourceFile == NULL || targetFile == NULL) {
        LOG_ERROR("Error: Source or target file is NULL\n");
        return;
    }
    LOG_INFO("Translating labels.\n");
    detectin = fopen(sourceFile, "r");
    if (detectin == NULL) {
        LOG_ERROR("Error: Could not open %s for reading\n", sourceFile);
        return;
    }
    detectlex();
    fclose(detectin);
    translatein = fopen(sourceFile, "r");
    if (translatein == NULL) {
        LOG_ERROR("Error: Could not open %s for reading\n", sourceFile);
        return;
    }
    outputFile = fopen(targetFile, "w");
    if (outputFile == NULL) {
        LOG_ERROR("Error: Could not open %s for writing\n", targetFile);
        return;
    }
    translatelex();
    fclose(translatein);
    fclose(outputFile);
    return;
}