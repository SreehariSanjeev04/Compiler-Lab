#include "codegen.h"
#include "expr_codegen.h"
#include <codegen_utils.h>
#include <symboltable.h>
#include <symboltable_utils.h>
#include <register_alloc.h>
#include <binding.h>

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

int codeGenLeafValue(tnode *root, FILE *targetFile)
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
        fprintf(stderr, "Error: Unknown leaf node type %d\n", root->nodetype);
        exit(1);
    }
}

int codeGenAddressExpr(tnode *root, FILE *targetFile)
{
    switch (root->nodetype)
    {
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
        fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
        exit(1);
    }
}

int codeGenBinaryOp(tnode *root, FILE *targetFile)
{
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