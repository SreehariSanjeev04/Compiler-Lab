#include <runtime.h>
#include <register_alloc.h>

void addHeader(FILE *targetFile)
{
    fprintf(targetFile, "0\n2056\n0\n0\n0\n0\n0\n0\n");
    fprintf(targetFile, "MOV SP, 8192\n"); // stack top, safely above static data (4096..4121)
    fprintf(targetFile, "CALL MAIN\n");
}

void printValue(int reg, FILE *targetFile)
{

    saveRegisters(targetFile);

    int tempReg = getReg();
    fprintf(targetFile, "MOV R%d, \"Write\"\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "MOV R%d, -2\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", reg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "CALL 0\n");
    fprintf(targetFile, "POP R%d\n", tempReg);
    fprintf(targetFile, "POP R%d\n", tempReg);
    fprintf(targetFile, "POP R%d\n", tempReg);
    fprintf(targetFile, "POP R%d\n", tempReg);
    fprintf(targetFile, "POP R%d\n", tempReg);

    freeReg();
    restoreRegisters(targetFile);
}

void readValue(int reg, FILE *targetFile)
{
    saveRegisters(targetFile);

    int tempReg = getReg();
    fprintf(targetFile, "MOV R%d, \"Read\"\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "MOV R%d, -1\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", reg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "CALL 0\n");
    fprintf(targetFile, "POP R%d\n", tempReg);
    fprintf(targetFile, "POP R%d\n", tempReg);
    fprintf(targetFile, "POP R%d\n", tempReg);
    fprintf(targetFile, "POP R%d\n", tempReg);
    fprintf(targetFile, "POP R%d\n", tempReg);

    freeReg();
    restoreRegisters(targetFile);
}

void exitProgram(FILE *targetFile)
{
    int tempReg = getReg();
    fprintf(targetFile, "MOV R%d, \"Exit\"\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "CALL 0\n");
    freeReg();
}
