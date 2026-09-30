#include "logger.h"
#include "codegen.h"
#include "stmt_codegen.h"
#include <codegen_utils.h>
#include <register_alloc.h>
#include <runtime.h>
#include <binding.h>
#include <lsymboltable.h>
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
        LOG_ERROR("Error: Unknown leaf node type %d\n", root->nodetype);
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
            LOG_ERROR("Error: If condition must be of boolean or integer type\n");
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
            LOG_ERROR("Error: If condition must be of boolean or integer type\n");
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
            LOG_ERROR("Error: While condition must be of boolean or integer type\n");
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
            LOG_ERROR("Error: Do-While condition must be of boolean or integer type\n");
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
            LOG_ERROR("Error: Repeat-Until condition must be of boolean or integer type\n");
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
        LOG_ERROR("Error: Unknown node type %d\n", root->nodetype);
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
             variableNode->nodetype != NODE_TYPE_DEREF) && 
             variableNode->nodetype != NODE_TYPE_TUPLE)
        {
            LOG_ERROR("Error: READ node must have an ID, array node, or dereference as its left child\n");
            exit(1);
        }
        int addressReg;
        if (variableNode->nodetype == NODE_TYPE_ID)
        {
            if (variableNode->Gentry != NULL && variableNode->Gentry->tupleEntry != NULL)
            {
                LOG_ERROR("Error: Cannot read a whole tuple; read individual fields instead\n");
                exit(1);
            }
            addressReg = getReg();
            emitVarAddressInto(addressReg, variableNode->varname, targetFile);
        }
        else if (variableNode->nodetype == NODE_TYPE_DEREF)
        {
            if (variableNode->pointerLevel != 0)
            {
                LOG_ERROR("Error: Can only read into a dereferenced integer location\n");
                exit(1);
            }
            addressReg = codeGenAddressOperand(variableNode->left, targetFile);
        }
        else if (variableNode->nodetype == NODE_TYPE_ARRAY)
        {
            addressReg = codeGenArrayAddress(variableNode, targetFile);
        } else {
            addressReg = codeGenTupleAddress(variableNode, targetFile);
        }
        readValue(addressReg, targetFile);
        freeReg();
        return -1;
    }
    case NODE_TYPE_RETURN:
    {
        if (root->left == NULL)
        {
            LOG_ERROR("Error: RETURN statement requires an expression\n");
            exit(1);
        }
        if (root->left->type == TYPE_TUPLE && root->left->pointerLevel == 0)
        {
            tnode *value = root->left;
            struct TupleTable *tupleType = value->Gentry ? value->Gentry->tupleEntry :
                                           value->Lentry ? value->Lentry->tupleEntry : NULL;
            if (value->nodetype != NODE_TYPE_ID || tupleType == NULL)
            {
                LOG_ERROR("Error: Tuple return requires a tuple variable\n");
                exit(1);
            }
            int sourceReg = getReg();
            emitVarAddressInto(sourceReg, value->varname, targetFile);
            int addressReg = getReg();
            int valueReg = getReg();
            for (int offset = 0; offset < tupleType->size; offset++)
            {
                fprintf(targetFile, "MOV R%d, [R%d]\n", valueReg, sourceReg);
                fprintf(targetFile, "MOV R%d, BP\n", addressReg);
                fprintf(targetFile, "SUB R%d, %d\n", addressReg, offset + 2);
                fprintf(targetFile, "MOV [R%d], R%d\n", addressReg, valueReg);
                if (offset + 1 < tupleType->size)
                    fprintf(targetFile, "ADD R%d, 1\n", sourceReg);
            }
            freeReg(); // valueReg
            freeReg(); // addressReg
            freeReg(); // sourceReg
        }
        else
        {
            int valueReg = codeGen(root->left, targetFile);
            // Store the return value at [BP-2], the slot reserved by the caller.
            int addressReg = getReg();
            fprintf(targetFile, "MOV R%d, BP\n", addressReg);
            fprintf(targetFile, "SUB R%d, 2\n", addressReg);
            fprintf(targetFile, "MOV [R%d], R%d\n", addressReg, valueReg);
            freeReg(); // addressReg
            freeReg(); // valueReg
        }
        // Deallocate the local variables, restore the caller's BP and return.
        int localCount = 0;
        for (struct Lsymbol* current = LSymbolGetHead(); current != NULL; current = current->next)
        {
            if (current->binding > 0)
            {
                localCount += (current->tupleEntry != NULL && current->pointerLevel == 0)
                              ? current->tupleEntry->size : 1;
            }
        }
        if (localCount > 0)
        {
            int countReg = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", countReg, localCount);
            fprintf(targetFile, "SUB SP, R%d\n", countReg);
            freeReg();
        }
        fprintf(targetFile, "POP BP\n");
        fprintf(targetFile, "RET\n");
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
            LOG_ERROR("ERROR: The types of the left and right operands in the assignment are not compatible\n");
            exit(1);
        }

        if (root->left->type == TYPE_TUPLE && root->left->pointerLevel == 0 &&
            root->right->nodetype == NODE_TYPE_FUNC_CALL)
        {
            if (root->left->nodetype != NODE_TYPE_ID)
            {
                LOG_ERROR("Error: Tuple call result requires a tuple variable destination\n");
                exit(1);
            }
            codeGenTupleCallInto(root->right, root->left, targetFile);
            return -1;
        }

        // Whole-tuple copy: b = a.
        if (root->left->type == TYPE_TUPLE && root->left->pointerLevel == 0 &&
            root->left->nodetype == NODE_TYPE_ID && root->right->nodetype == NODE_TYPE_ID)
        {
            struct TupleTable *tupleType = root->left->Gentry ? root->left->Gentry->tupleEntry :
                                           root->left->Lentry->tupleEntry;
            int dstReg = getReg();
            emitVarAddressInto(dstReg, root->left->varname, targetFile);
            int srcReg = getReg();
            emitVarAddressInto(srcReg, root->right->varname, targetFile);
            int tmpReg = getReg();
            for (int offset = 0; offset < tupleType->size; offset++)
            {
                fprintf(targetFile, "MOV R%d, [R%d]\n", tmpReg, srcReg);
                fprintf(targetFile, "MOV [R%d], R%d\n", dstReg, tmpReg);
                if (offset < tupleType->size - 1)
                {
                    fprintf(targetFile, "ADD R%d, 1\n", srcReg);
                    fprintf(targetFile, "ADD R%d, 1\n", dstReg);
                }
            }
            freeReg(); // tmpReg
            freeReg(); // srcReg
            freeReg(); // dstReg
            return -1;
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
            int addressReg = getReg();
            emitVarAddressInto(addressReg, root->left->varname, targetFile);
            fprintf(targetFile, "MOV [R%d], R%d\n", addressReg, rightReg);
            freeReg(); // free addressReg
        }
        else if (root->left->nodetype == NODE_TYPE_DEREF)
        {
            // The address of *p is the value of p.
            int addressReg = codeGenAddressOperand(root->left->left, targetFile);
            fprintf(targetFile, "MOV [R%d], R%d\n", addressReg, rightReg);
            freeReg(); // free addressReg
        }
        else if (root->left->nodetype == NODE_TYPE_TUPLE)
        {
            int addressReg = codeGenTupleAddress(root->left, targetFile);
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
        LOG_ERROR("Error: Unknown node type %d\n", root->nodetype);
        exit(1);
    }
}
