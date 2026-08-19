#include <flabel.h>

int functionLabelCounter = 0;

int generateFunctionLabel() {
    return functionLabelCounter++;
}