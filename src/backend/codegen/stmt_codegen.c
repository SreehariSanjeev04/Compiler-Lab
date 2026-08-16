#include "codegen.h"
#include "stmt_codegen.h"
#include <codegen_utils.h>
#include <register_alloc.h>
#include <runtime.h>
#include <binding.h>
#include "expr_codegen.h"

static bool insideWhileLoop = false;
static int whileStartLabel = -1;
static int whileEndLabel = -1;

int codeGenControlLeaf(tnode *root, FILE *targetFile)
{
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

int codeGenFlowControl(tnode *root, FILE *targetFile)
{
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
    default:
        fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
        exit(1);
    }
}

int codeGenStatement(tnode *root, FILE *targetFile)
{
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
        fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
        exit(1);
    }
}