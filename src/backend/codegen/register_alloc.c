#include <register_alloc.h>
#include <stdlib.h>
#include <constants.h>

int regCount = 0;
int labelCount = 0;

int getReg(void)
{
    if (regCount < MAX_REGISTERS)
    {
        return regCount++;
    }
    else
    {
        fprintf(stderr, "Error: Out of registers\n");
        exit(1);
    }
}

int generateLabel(void)
{
    return labelCount++;
}

int getRegCount(void)
{
    return regCount;
}

void freeReg(void)
{
    if (regCount > 0)
    {
        regCount--;
    }
    else
    {
        fprintf(stderr, "Error: No registers to free\n");
        exit(1);
    }
}

void saveRegisters(FILE *targetFile)
{
    for (int i = 0; i < regCount; i++)
    {
        fprintf(targetFile, "PUSH R%d\n", i);
    }
}

void restoreRegisters(FILE *targetFile)
{
    for (int i = regCount - 1; i >= 0; i--)
    {
        fprintf(targetFile, "POP R%d\n", i);
    }
}
