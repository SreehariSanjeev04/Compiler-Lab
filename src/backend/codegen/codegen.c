#include "codegen.h"
#include <runtime.h>
#include "expr_codegen.h"
#include "stmt_codegen.h"
/**
 * To do: Check the codegen function to make sure everything is alright
 */

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
        case NODE_TYPE_STRING:
        case NODE_TYPE_ID:
            return codeGenLeafValue(root, targetFile);
        case NODE_TYPE_BREAKPOINT:
        case NODE_TYPE_BREAK:
        case NODE_TYPE_CONTINUE:
            return codeGenControlLeaf(root, targetFile);
        default:
            fprintf(stderr, "Error: Unknown leaf node type %d\n", root->nodetype);
            exit(1);
        }
    }

    // Cases that require special statement handling: If, If Else, While, Array
    switch (root->nodetype)
    {
    case NODE_TYPE_IF:
    case NODE_TYPE_IF_ELSE:
    case NODE_TYPE_WHILE:
    case NODE_TYPE_DO_WHILE:
    case NODE_TYPE_REPEAT_UNTIL:
        return codeGenFlowControl(root, targetFile);
    case NODE_TYPE_ARRAY:
    case NODE_TYPE_ADDRESS:
    case NODE_TYPE_DEREF:
        return codeGenAddressExpr(root, targetFile);
    default:
        break;
    }

    // Cases that require special statement handling: READ, WRITE, and ASSIGN
    switch (root->nodetype)
    {
    case NODE_TYPE_READ:
    case NODE_TYPE_WRITE:
    case NODE_TYPE_ASSIGN:
        return codeGenStatement(root, targetFile);
    default:
        break;
    }

    return codeGenBinaryOp(root, targetFile);
}