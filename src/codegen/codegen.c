#include "codegen.h"
int regCount = 0;

void addHeader(FILE* targetFile) {
    fprintf(targetFile, "0\n2056\n0\n0\n0\n0\n0\n0\n");
    fprintf(targetFile, "MOV SP, 4121\n");
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

int returnStaticBindAddress(char* varname) {
    if(!varname || strlen(varname) != 1) {
        fprintf(stderr, "Error: Variable name is invalid. Only single-letter variable names are allowed.\n");
        exit(1);
    }
    if (varname[0] >= 'a' && varname[0] <= 'z') {
        return 4096 + (varname[0] - 'a');
    } else {
        fprintf(stderr, "Error: Invalid variable name %s\n", varname);
        exit(1);
    }
}

void generateCode(tnode* root, FILE* targetFile) {
    if(root == NULL) {
        fprintf(stderr, "Error: Syntax tree is empty. Cannot generate code.\n");
        exit(1);
    }
    addHeader(targetFile);
    codeGen(root, targetFile);
    exitProgram(targetFile);
    printf("Code generation completed successfully.\n");
}

int codeGen(tnode* root, FILE* targetFile) {
    if (root == NULL) {
        return -1; 
    }

    if(root->nodetype == NODE_TYPE_CONNECTOR) {
        codeGen(root->left, targetFile);
        codeGen(root->right, targetFile);
        return -1; 
    }

    // Only possible leaf nodes are NUM and ID
    if (!root->left && !root->right) {
        int reg = getReg();
        if (root->nodetype == NODE_TYPE_NUM) {
            fprintf(targetFile, "MOV R%d, %d\n", reg, root->val);
        } else if (root->nodetype == NODE_TYPE_ID) {
            int address = returnStaticBindAddress(root->varname);
            fprintf(targetFile, "MOV R%d, [%d]\n", reg, address);
        }
        return reg;
    }

    // Cases that require special statement handling: READ, WRITE, and ASSIGN
    switch (root->nodetype)
    {
        case NODE_TYPE_READ: {
            int addressReg = getReg();
            tnode* variableNode = root->left;
            if (variableNode->nodetype == NODE_TYPE_PLUS || variableNode->nodetype != NODE_TYPE_ID) {
                fprintf(stderr, "Error: READ node must have an ID as its left child\n");
                exit(1);
            }
            int address = returnStaticBindAddress(variableNode->varname);
            fprintf(targetFile, "MOV R%d, %d\n", addressReg, address);
            readValue(addressReg, targetFile);
            freeReg(); 
            return -1; 
        }
        case NODE_TYPE_WRITE: {
            int valueReg = codeGen(root->left, targetFile);
            printValue(valueReg, targetFile);
            freeReg(); 
            return -1; 
        }
        case NODE_TYPE_ASSIGN: {
            int rightReg = codeGen(root->right, targetFile);
            int address = returnStaticBindAddress(root->left->varname);
            fprintf(targetFile, "MOV [%d], R%d\n", address, rightReg);
            freeReg(); 
            return -1;
        }
        default:
            break;
    }

    int leftReg = codeGen(root->left, targetFile);
    int rightReg = codeGen(root->right, targetFile);

    switch (root->nodetype) {
        case NODE_TYPE_PLUS:
            fprintf(targetFile, "ADD R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_TYPE_MINUS:
            fprintf(targetFile, "SUB R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_TYPE_MUL:
            fprintf(targetFile, "MUL R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_TYPE_DIV:
            fprintf(targetFile, "DIV R%d, R%d\n", leftReg, rightReg);
            break;
        default:
            fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
            exit(1);
    }

    freeReg(); 
    return leftReg; 
}

void printValue(int reg, FILE* targetFile) {
    
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

void readValue(int reg, FILE* targetFile) {
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

void exitProgram(FILE* targetFile) {
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