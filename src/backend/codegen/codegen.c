#include "codegen.h"
#include <symboltable.h>
int regCount = 0;
int labelCount = 0;
/**
 * To do: Check the codegen function to make sure everything is alright
 */

static bool insideWhileLoop = false;
static int whileStartLabel = -1;
static int whileEndLabel = -1;

void addHeader(FILE *targetFile)
{
    fprintf(targetFile, "0\n2056\n0\n0\n0\n0\n0\n0\n");
    fprintf(targetFile, "MOV SP, 4121\n");
}

int getReg()
{
    if (regCount < 20)
    {
        return regCount++;
    }
    else
    {
        fprintf(stderr, "Error: Out of registers\n");
        exit(1);
    }
}

int generateLabel()
{
    return labelCount++;
}

void freeReg()
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

int returnStaticBindAddress(char *varname)
{
    if (!varname || strlen(varname) <= 0)
    {
        fprintf(stderr, "Error: Variable name is invalid. Only single-letter variable names are allowed.\n");
        exit(1);
    }
    struct Gsymbol *symbol = Lookup(varname);
    if (!symbol)
    {
        fprintf(stderr, "Error: Variable '%s' not defined\n", varname);
        exit(1);
    }
    return symbol->binding;
}

void generateCode(tnode *root, FILE *targetFile)
{
    if (root == NULL)
    {
        fprintf(stderr, "Error: Syntax tree is empty. Cannot generate code.\n");
        exit(1);
    }
    addHeader(targetFile);
    codeGen(root, targetFile);
    exitProgram(targetFile);
    printf("Code generation completed successfully.\n");
}

int codeGen(tnode *root, FILE *targetFile)
{
    if (root == NULL)
    {
        return -1;
    }

    if (root->nodetype == NODE_TYPE_CONNECTOR)
    {
        codeGen(root->left, targetFile);
        codeGen(root->right, targetFile);
        return -1;
    }

    // Only possible leaf nodes are NUM and ID
    if (!root->left && !root->right)
    {
        int reg = getReg();
        if (root->nodetype == NODE_TYPE_NUM)
        {
            fprintf(targetFile, "MOV R%d, %d\n", reg, root->val);
        }
        else if (root->nodetype == NODE_TYPE_STRING)
        {
            fprintf(targetFile, "MOV R%d, %s\n", reg, root->varname);
        }
        else if (root->nodetype == NODE_TYPE_ID)
        {
            int address = returnStaticBindAddress(root->varname);
            fprintf(targetFile, "MOV R%d, %d\n", reg, address);   
        }
        else if (root->nodetype == NODE_TYPE_BREAKPOINT)
        {
            fprintf(targetFile, "BRKP\n");
        }
        else if (root->nodetype == NODE_TYPE_BREAK && insideWhileLoop)
        {
            fprintf(targetFile, "JMP L%d\n", whileEndLabel);
        }
        else if (root->nodetype == NODE_TYPE_CONTINUE && insideWhileLoop)
        {
            fprintf(targetFile, "JMP L%d\n", whileStartLabel);
        }
        else
        {
            fprintf(stderr, "Error: Unknown leaf node type %d\n", root->nodetype);
            exit(1);
        }
        return reg;
    }
    // Cases that require special statement handling: If, If Else, While, Array
    switch (root->nodetype)
    {
    case NODE_TYPE_IF:
    {
        if (root->left->type != TYPE_BOOL)
        {
            fprintf(stderr, "Error: If condition must be of boolean type\n");
            exit(1);
        }
        int conditionReg = codeGen(root->left, targetFile);
        int labelElse = generateLabel();
        fprintf(targetFile, "JZ R%d, L%d\n", conditionReg, labelElse);
        freeReg();
        codeGen(root->right, targetFile);
        fprintf(targetFile, "L%d:\n", labelElse);
        return -1;
    }
    case NODE_TYPE_IF_ELSE:
    {
        if (root->left->type != TYPE_BOOL)
        {
            fprintf(stderr, "Error: If condition must be of boolean type\n");
            exit(1);
        }
        int conditionReg = codeGen(root->left, targetFile);
        int labelElse = generateLabel();
        int labelEnd = generateLabel();
        fprintf(targetFile, "JZ R%d, L%d\n", conditionReg, labelElse);
        freeReg();
        codeGen(root->right, targetFile); // then block
        fprintf(targetFile, "JMP L%d\n", labelEnd);
        fprintf(targetFile, "L%d:\n", labelElse);
        codeGen(root->middle, targetFile); // else block
        fprintf(targetFile, "L%d:\n", labelEnd);
        return -1;
    }
    case NODE_TYPE_WHILE:
    {
        if (root->left->type != TYPE_BOOL)
        {
            fprintf(stderr, "Error: While condition must be of boolean type\n");
            exit(1);
        }
        insideWhileLoop = true;
        int labelStart = generateLabel();
        int labelEnd = generateLabel();
        int prevStart = whileStartLabel;
        int prevEnd = whileEndLabel;
        whileStartLabel = labelStart;
        whileEndLabel = labelEnd;
        fprintf(targetFile, "L%d:\n", labelStart);
        int conditionReg = codeGen(root->left, targetFile);
        fprintf(targetFile, "JZ R%d, L%d\n", conditionReg, labelEnd);
        freeReg();
        codeGen(root->right, targetFile); // body of while
        fprintf(targetFile, "JMP L%d\n", labelStart);
        fprintf(targetFile, "L%d:\n", labelEnd);
        whileStartLabel = prevStart;
        whileEndLabel = prevEnd;
        insideWhileLoop = false;
        return -1;
    }
    case NODE_TYPE_DO_WHILE:
    {
        if (root->right->type != TYPE_BOOL)
        {
            fprintf(stderr, "Error: Do-While condition must be of boolean type\n");
            exit(1);
        }
        insideWhileLoop = true;
        int labelStart = generateLabel();
        int labelEnd = generateLabel();
        int prevStart = whileStartLabel;
        int prevEnd = whileEndLabel;
        whileStartLabel = labelStart;
        whileEndLabel = labelEnd;
        fprintf(targetFile, "L%d:\n", labelStart);
        codeGen(root->left, targetFile); // body of do-while
        int conditionReg = codeGen(root->right, targetFile);
        fprintf(targetFile, "JNZ R%d, L%d\n", conditionReg, labelStart);
        freeReg();
        fprintf(targetFile, "L%d:\n", labelEnd);
        whileStartLabel = prevStart;
        whileEndLabel = prevEnd;
        insideWhileLoop = false;
        return -1;
    }
    case NODE_TYPE_REPEAT_UNTIL:
    {
        if (root->right->type != TYPE_BOOL)
        {
            fprintf(stderr, "Error: Repeat-Until condition must be of boolean type\n");
            exit(1);
        }
        insideWhileLoop = true;
        int labelStart = generateLabel();
        int labelEnd = generateLabel();
        int prevStart = whileStartLabel;
        int prevEnd = whileEndLabel;
        whileStartLabel = labelStart;
        whileEndLabel = labelEnd;
        fprintf(targetFile, "L%d:\n", labelStart);
        codeGen(root->left, targetFile); // body of repeat-until
        int conditionReg = codeGen(root->right, targetFile);
        fprintf(targetFile, "JZ R%d, L%d\n", conditionReg, labelStart);
        freeReg();
        fprintf(targetFile, "L%d:\n", labelEnd);
        whileStartLabel = prevStart;
        whileEndLabel = prevEnd;
        insideWhileLoop = false;
        return -1;
    }
    case NODE_TYPE_ARRAY:
    {
        // the leftmost child would be the variable and the rightmost child would be the last index
        tnode* current = root;
        while(current->left != NULL) {
            current = current->left;
        }
        if(current->nodetype != NODE_TYPE_ID) {
            fprintf(stderr, "Error: Array node must have an ID as its leftmost child\n");
            exit(1);
        }
        struct Gsymbol *symbol = Lookup(current->varname);
        if(symbol == NULL) {
            fprintf(stderr, "Error: Variable '%s' not defined\n", current->varname);
            exit(1);
        }
        int numOfDimensions = symbol->dimensions;
        int bindingAddress = returnStaticBindAddress(current->varname);
        int* strideArray = createStrideArray(symbol);
        int totalOffsetReg = getReg();
        fprintf(targetFile, "MOV R%d, %d\n", totalOffsetReg, bindingAddress); // initialize total offset with binding address
        int indexCount = numOfDimensions - 1;
        tnode* current = root;
        while(current->left != NULL) {
            int computeReg = getReg();
            int indexReg = codeGen(current->right, targetFile);
            fprintf(targetFile, "MOV R%d, %d\n", computeReg, strideArray[indexCount]);
            fprintf(targetFile, "MUL R%d, R%d\n", computeReg, indexReg);
            fprintf(targetFile, "ADD R%d, R%d\n", totalOffsetReg, computeReg);
            freeReg(); // free computeReg
            freeReg(); // free indexReg
            current = current->left;
            indexCount--;
        }
        free(strideArray);
        return totalOffsetReg;
    }
    default:
        break;
    }

    // Cases that require special statement handling: READ, WRITE, and ASSIGN
    switch (root->nodetype)
    {
    case NODE_TYPE_READ:
    {
        tnode *variableNode = root->left;
        if (variableNode->nodetype != NODE_TYPE_ID   && variableNode->nodetype != NODE_TYPE_ARRAY)
        {
            fprintf(stderr, "Error: READ node must have an ID or array node as its left child\n");
            exit(1);
        }
        int addressReg = codeGen(variableNode, targetFile);
        readValue(addressReg, targetFile);
        freeReg();
        return -1;
    }
    case NODE_TYPE_WRITE:
    {
        int addressReg = codeGen(root->left, targetFile);
        fprintf(targetFile, "MOV R%d, [R%d]\n", addressReg, addressReg); // load the value from the address into the register
        printValue(addressReg, targetFile);
        freeReg();
        return -1;
    }
    case NODE_TYPE_ASSIGN:
    {
        if (!root->left || (root->left->nodetype != NODE_TYPE_ID && root->left->nodetype != NODE_TYPE_ARRAY))
        {
            fprintf(stderr, "Error: ASSIGN node must have an ID or array node as its left child\n");
            exit(1);
        }
        int rightReg = codeGen(root->right, targetFile);
        int address = returnStaticBindAddress(root->left->varname);
        fprintf(targetFile, "MOV [%d], R%d\n", address, rightReg);
        freeReg();
        return -1;
    }
    default:
        break;
    }

    // below this is arithmetic and relational operations, which are binary operations
    int leftReg = codeGen(root->left, targetFile);
    int rightReg = codeGen(root->right, targetFile);

    if(root->left->type != TYPE_INT || root->right->type != TYPE_INT) {
        fprintf(stderr, "Error: Type Mismatch in arithmetic or relational operation\n");
        exit(1);
    }
    switch (root->nodetype)
    {
    case NODE_TYPE_PLUS:
    {
        fprintf(targetFile, "ADD R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_MINUS:
    {
        fprintf(targetFile, "SUB R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_MUL: {
        fprintf(targetFile, "MUL R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_DIV: {
        fprintf(targetFile, "DIV R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_GT: {
        fprintf(targetFile, "GT R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_LT: {
        fprintf(targetFile, "LT R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_GE: {
        fprintf(targetFile, "GE R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_LE: {
        fprintf(targetFile, "LE R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_EQ:
        fprintf(targetFile, "EQ R%d, R%d\n", leftReg, rightReg);
        break;
    case NODE_TYPE_NE: // have to check this out
        fprintf(targetFile, "NE R%d, R%d\n", leftReg, rightReg);
        break;
    default:
        fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
        exit(1);
    }

    freeReg();
    return leftReg;
}

/*
* This function generates the assembly code to print a value stored in a register.
* @param reg: The register number that contains the value to be printed
* @param targetFile: The file pointer to the target file where the code is being generated
* @return: void
*/
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

/*
* This function generates the assembly code to read a value from the user and store it in a specified register.
* @param reg: The register number where the read value will be stored
* @param targetFile: The file pointer to the target file where the code is being generated
* @return: void
*/
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

/**
 * This function generates the assembly code to exit the program.
 * @param targetFile: The file pointer to the target file where the code is being generated
 */
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