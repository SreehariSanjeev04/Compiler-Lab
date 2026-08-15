#include "codegen.h"
#include <codegen_utils.h>
#include <symboltable.h>
#include <utils.h>
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

int returnStaticBindAddress(const char *varname)
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

/*
 * Computes the memory address of an array element into a register.
 * The array node is a left-leaning tree: the leftmost leaf is the variable
 * name and the right child of each node is the index for that dimension.
 * @param root: The NODE_TYPE_ARRAY node whose address is to be computed
 * @param targetFile: The file pointer to the target file where the code is being generated
 * @return: The register number holding the element address
 */
int codeGenArrayAddress(tnode *root, FILE *targetFile)
{
    tnode *current = root;
    while (current->left != NULL)
    {
        current = current->left;
    }
    if (current->nodetype != NODE_TYPE_ID)
    {
        fprintf(stderr, "Error: Array node must have an ID as its leftmost child\n");
        exit(1);
    }
    struct Gsymbol *symbol = Lookup(current->varname);
    if (symbol == NULL)
    {
        fprintf(stderr, "Error: Variable '%s' not defined\n", current->varname);
        exit(1);
    }
    int bindingAddress = returnStaticBindAddress(current->varname);
    int *strideArray = createStrideArray(symbol);

    // Start the total offset with the binding address of the array
    int totalOffsetReg = getReg();
    fprintf(targetFile, "MOV R%d, %d\n", totalOffsetReg, bindingAddress);

    // The index at tree depth j (1 = rightmost) corresponds to dimension
    // (numIndices - j), where numIndices is the number of index nodes present.
    // With fewer indices than dimensions, the missing inner dimensions default
    // to 0, so the first stride used must be that of the outermost index.
    int numIndices = 0;
    current = root;
    while (current->left != NULL)
    {
        numIndices++;
        current = current->left;
    }
    int indexCount = numIndices - 1;
    current = root;
    while (current->left != NULL)
    {
        int computeReg = getReg();
        int indexReg = codeGen(current->right, targetFile);
        fprintf(targetFile, "MOV R%d, %d\n", computeReg, strideArray[indexCount]);
        fprintf(targetFile, "MUL R%d, R%d\n", computeReg, indexReg);
        fprintf(targetFile, "ADD R%d, R%d\n", totalOffsetReg, computeReg);
        freeReg(); // free indexReg (allocated last)
        freeReg(); // free computeReg
        current = current->left;
        indexCount--;
    }
    free(strideArray);
    return totalOffsetReg;
}

/*
 * Produces the value of an expression that is being used as a pointer/address,
 * applying array decay: an array element expression yields its element address
 * and an array name yields its base address, without any memory load. Other
 * expressions are evaluated normally (an ID pointer loads its stored address).
 * @param root: The node whose pointer value is to be produced
 * @param targetFile: The file pointer to the target file where the code is being generated
 * @return: The register number holding the pointer value (caller must free it)
 */
int codeGenAddressOperand(tnode *root, FILE *targetFile)
{
    if (root->nodetype == NODE_TYPE_ARRAY)
    {
        return codeGenArrayAddress(root, targetFile);
    }
    if (root->nodetype == NODE_TYPE_ID && root->Gentry != NULL && root->Gentry->dimensions > 0)
    {
        int reg = getReg();
        fprintf(targetFile, "MOV R%d, %d\n", reg, root->Gentry->binding);
        return reg;
    }
    return codeGen(root, targetFile);
}

/*
 * Generates XSM assembly for a syntax tree node.
 * Value-producing nodes return the register number holding the result;
 * statement nodes return -1. The caller must free the returned register
 * (except -1) with freeReg() once it is no longer needed.
 */
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

    // Leaf nodes: NUM, ID, STRING load values into registers.
    if (!root->left && !root->right)
    {
        switch (root->nodetype)
        {
        case NODE_TYPE_NUM:
        {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", reg, root->val);
            return reg;
        }
        case NODE_TYPE_STRING:
        {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, %s\n", reg, root->varname);
            return reg;
        }
        case NODE_TYPE_ID:
        {
            // Dereference the variable: load its value into the register.
            int address = returnStaticBindAddress(root->varname);
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, [%d]\n", reg, address);
            return reg;
        }
        default:
            // Statement-only leaves (breakpoint, break, continue) do not use a register.
            if (root->nodetype == NODE_TYPE_BREAKPOINT)
            {
                fprintf(targetFile, "BRKP\n");
            }
            else if (root->nodetype == NODE_TYPE_BREAK)
            {
                if (insideWhileLoop)
                {
                    fprintf(targetFile, "JMP L%d\n", whileEndLabel);
                }
            }
            else if (root->nodetype == NODE_TYPE_CONTINUE)
            {
                if (insideWhileLoop)
                {
                    fprintf(targetFile, "JMP L%d\n", whileStartLabel);
                }
            }
            else
            {
                fprintf(stderr, "Error: Unknown leaf node type %d\n", root->nodetype);
                exit(1);
            }
            return -1;
        }
    }
    // Cases that require special statement handling: If, If Else, While, Array
    switch (root->nodetype)
    {
    case NODE_TYPE_IF:
    {
        if (root->left->type != TYPE_BOOL && root->left->type != TYPE_INT)
        {
            fprintf(stderr, "Error: If condition must be of boolean or integer type\n");
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
        if (root->left->type != TYPE_BOOL && root->left->type != TYPE_INT)
        {
            fprintf(stderr, "Error: If condition must be of boolean or integer type\n");
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
        if (root->left->type != TYPE_BOOL && root->left->type != TYPE_INT)
        {
            fprintf(stderr, "Error: While condition must be of boolean or integer type\n");
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
        if (root->right->type != TYPE_BOOL && root->right->type != TYPE_INT)
        {
            fprintf(stderr, "Error: Do-While condition must be of boolean or integer type\n");
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
        if (root->right->type != TYPE_BOOL && root->right->type != TYPE_INT)
        {
            fprintf(stderr, "Error: Repeat-Until condition must be of boolean or integer type\n");
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
        // Used as a value in an expression: compute the element address, then dereference.
        int reg = codeGenArrayAddress(root, targetFile);
        fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
        return reg;
    }
    case NODE_TYPE_ADDRESS:
    {
        if (root->left->nodetype != NODE_TYPE_ID && root->left->nodetype != NODE_TYPE_ARRAY)
        {
            fprintf(stderr, "Error: Address node must have an ID or an array node as its left child\n");
            exit(1);
        }
        if (root->left->nodetype == NODE_TYPE_ARRAY)
            return codeGenArrayAddress(root->left, targetFile); // computes the base address + offset

        int address = returnStaticBindAddress(root->left->varname);
        int reg = getReg();
        fprintf(targetFile, "MOV R%d, %d\n", reg, address); // load the address to the register
        return reg;
    }
    case NODE_TYPE_DEREF:
    {
        if (effectivePointerLevel(root->left) <= 0)
        {
            fprintf(stderr, "Error: Cannot dereference a non-pointer expression\n");
            exit(1);
        }
        if (root->left->nodetype == NODE_TYPE_ARRAY ||
            (root->left->nodetype == NODE_TYPE_ID && root->left->Gentry != NULL && root->left->Gentry->dimensions > 0))
        {
            // Array operand: *arr is the first element of arr. If that element
            // is itself an array (remaining dimensions), it decays to its
            // address and no load is performed; otherwise one load yields it.
            int reg = codeGenAddressOperand(root->left, targetFile);
            if (root->pointerLevel > 0)
            {
                return reg;
            }
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
            return reg;
        }
        int reg = codeGen(root->left, targetFile);
        fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
        return reg;
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
        if (variableNode == NULL ||
            (variableNode->nodetype != NODE_TYPE_ID &&
             variableNode->nodetype != NODE_TYPE_ARRAY &&
             variableNode->nodetype != NODE_TYPE_DEREF))
        {
            fprintf(stderr, "Error: READ node must have an ID, array node, or dereference as its left child\n");
            exit(1);
        }
        int addressReg;
        if (variableNode->nodetype == NODE_TYPE_ID)
        {
            int address = returnStaticBindAddress(variableNode->varname);
            addressReg = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", addressReg, address);
        }
        // allowing reading into dereferenced pointers as well, but only if the pointer level is 0
        else if (variableNode->nodetype == NODE_TYPE_DEREF)
        {
            if (variableNode->pointerLevel != 0)
            {
                fprintf(stderr, "Error: Can only read into a dereferenced integer location\n");
                exit(1);
            }
            addressReg = codeGenAddressOperand(variableNode->left, targetFile);
        }
        else
        {
            addressReg = codeGenArrayAddress(variableNode, targetFile);
        }
        readValue(addressReg, targetFile);
        freeReg();
        return -1;
    }
    case NODE_TYPE_WRITE:
    {
        int valueReg = codeGen(root->left, targetFile);
        printValue(valueReg, targetFile);
        freeReg();
        return -1;
    }
    case NODE_TYPE_ASSIGN:
    {
        if (!isAssignmentCompatible(root->left, root->right))
        {
            fprintf(stderr, "ERROR: The types of the left and right operands in the assignment are not compatible\n");
            exit(1);
        }

        int rightReg;
        // Array decay on the RHS: when storing into a pointer, an array name
        // or array element yields its address instead of its value.
        if (effectivePointerLevel(root->left) > 0 &&
            (root->right->nodetype == NODE_TYPE_ARRAY ||
             (root->right->nodetype == NODE_TYPE_ID && root->right->Gentry != NULL && root->right->Gentry->dimensions > 0)))
        {
            rightReg = codeGenAddressOperand(root->right, targetFile);
        }
        else
        {
            rightReg = codeGen(root->right, targetFile);
        }

        if (root->left->nodetype == NODE_TYPE_ID)
        {
            int address = returnStaticBindAddress(root->left->varname);
            fprintf(targetFile, "MOV [%d], R%d\n", address, rightReg);
        }
        else if (root->left->nodetype == NODE_TYPE_DEREF)
        {
            // The address of *p is the value of p.
            int addressReg = codeGenAddressOperand(root->left->left, targetFile);
            fprintf(targetFile, "MOV [R%d], R%d\n", addressReg, rightReg);
            freeReg(); // free addressReg
        }
        else
        {
            int addressReg = codeGenArrayAddress(root->left, targetFile);
            fprintf(targetFile, "MOV [R%d], R%d\n", addressReg, rightReg);
            freeReg(); // free addressReg
        }
        freeReg(); // free rightReg
        return -1;
    }
    default:
        break;
    }

    // below this is arithmetic and relational operations, which are binary operations
    if (!isArithmeticCompatible(root->left, root->right, root->nodetype))
    {
        fprintf(stderr, "Error: Type Mismatch in arithmetic or relational operation\n");
        exit(1);
    }
    // Pointer operands are fetched in address form (arrays decay to addresses),
    // so that p + 1 advances p by one word and p - q yields the word difference.
    int leftLevel = effectivePointerLevel(root->left);
    int rightLevel = effectivePointerLevel(root->right);
    int leftReg = leftLevel > 0 ? codeGenAddressOperand(root->left, targetFile) : codeGen(root->left, targetFile);
    int rightReg = rightLevel > 0 ? codeGenAddressOperand(root->right, targetFile) : codeGen(root->right, targetFile);
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
    case NODE_TYPE_MUL:
    {
        fprintf(targetFile, "MUL R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_DIV:
    {
        fprintf(targetFile, "DIV R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_GT:
    {
        fprintf(targetFile, "GT R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_LT:
    {
        fprintf(targetFile, "LT R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_GE:
    {
        fprintf(targetFile, "GE R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_LE:
    {
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