#include "codegen.h"
int regCount = 0;

void addHeader(FILE* targetFile) {
    fprintf(targetFile, "0\n2056\n0\n0\n0\n0\n0\n0\n");
}

int getReg() {
    if (regCount < 20) {
        return regCount++;
    } else {
        fprintf(stderr, "Error: Out of registers\n");
        exit(1);
    }
}

void freeReg() {
    if (regCount > 0) {
        regCount--;
    } else {
        fprintf(stderr, "Error: No registers to free\n");
        exit(1);
    }
}

void saveRegisters(FILE* targetFile) {
    for (int i = 0; i < regCount; i++) {
        fprintf(targetFile, "PUSH R%d\n", i);
    }
}

void restoreRegisters(FILE* targetFile) {
    for (int i = regCount - 1; i >= 0; i--) {
        fprintf(targetFile, "POP R%d\n", i);
    }
}
int codeGen(tnode* root, FILE* targetFile) {
    if(root == NULL) {
        return -1; // Return -1 for NULL nodes
    }

    if(!root->left && !root->right) {
        int reg = getReg();
        fprintf(targetFile, "MOV R%d, %d\n", reg, root->val);
        return reg;
    }
    int leftReg = codeGen(root->left, targetFile);
    int rightReg = codeGen(root->right, targetFile);

    if(root->op) {
        switch(*(root->op)) {
            case '+':
                fprintf(targetFile, "ADD R%d, R%d\n", leftReg, rightReg);
                freeReg();
                return leftReg;
            case '-':
                fprintf(targetFile, "SUB R%d, R%d\n", leftReg, rightReg);
                freeReg();
                return leftReg;
            case '*':
                fprintf(targetFile, "MUL R%d, R%d\n", leftReg, rightReg);
                freeReg();
                return leftReg;
            case '/':
                fprintf(targetFile, "DIV R%d, R%d\n", leftReg, rightReg);
                freeReg();
                return leftReg;
            default:
                fprintf(stderr, "Error: Unknown operator %c\n", *(root->op));
                exit(1);
        }
    }
    freeReg(); // free the right register
    return leftReg; // return the left register
}

void printValue(int reg, FILE* targetFile) {
    int tempReg = getReg();
    saveRegisters(targetFile);
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
    restoreRegisters(targetFile);
    freeReg(); // Free the temporary register
}

void exitProgram(FILE* targetFile) {
    int tempReg = getReg();
    fprintf(targetFile, "MOV R%d, \"Exit\"\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "PUSH R%d\n", tempReg);
    fprintf(targetFile, "CALL 0\n");
}