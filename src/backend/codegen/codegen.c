#include "logger.h"
#include "codegen.h"
#include <runtime.h>
#include <gsymboltable.h>
#include <lsymboltable.h>
#include "expr_codegen.h"
#include "stmt_codegen.h"

/*
 * XSM calling convention (per the stage-5 roadmap):
 *   Caller: save regs in use, evaluate arguments in reverse order and push
 *   them, push one empty slot for the return value, CALL <label>.
 *   Callee: PUSH BP, MOV BP SP, allocate locals; return value goes to [BP-2];
 *   epilogue pops locals, restores BP, RETs.
 */

static int countLocals(void) {
    int count = 0;
    struct Lsymbol* current = LSymbolGetHead();
    while (current != NULL) {
        if (current->binding > 0) {
            count += (current->tupleEntry != NULL && current->pointerLevel == 0)
                     ? current->tupleEntry->size : 1;
        }
        current = current->next;
    }
    return count;
}

static void emitFunctionLabel(struct Gsymbol* funcSymbol, FILE* targetFile) {
    if (strcmp(funcSymbol->name, "MAIN") == 0) {
        fprintf(targetFile, "MAIN:\n");
    } else {
        fprintf(targetFile, "F%d:\n", funcSymbol->flabel);
    }
}

static void emitPrologue(int localCount, FILE* targetFile) {
    fprintf(targetFile, "PUSH BP\n");
    fprintf(targetFile, "MOV BP, SP\n");
    if (localCount > 0) {
        fprintf(targetFile, "MOV R0, %d\n", localCount);
        fprintf(targetFile, "ADD SP, R0\n");
    }
}

/* Fallback epilogue used when control reaches the end of the body without a
 * RETURN: stores 0 as the return value. Uses raw registers R0/R1 outside the
 * register allocator since the function body generation has finished. */
static void emitEpilogue(int localCount, FILE* targetFile) {
    fprintf(targetFile, "MOV R0, 0\n");
    fprintf(targetFile, "MOV R1, BP\n");
    fprintf(targetFile, "SUB R1, 2\n");
    fprintf(targetFile, "MOV [R1], R0\n");
    if (localCount > 0) {
        fprintf(targetFile, "MOV R0, %d\n", localCount);
        fprintf(targetFile, "SUB SP, R0\n");
    }
    fprintf(targetFile, "POP BP\n");
    fprintf(targetFile, "RET\n");
}

void generateProgramStart(FILE* targetFile) {
    static bool programStarted = false;
    if (programStarted) {
        return;
    }
    programStarted = true;
    fprintf(targetFile, "0\n2056\n0\n0\n0\n0\n0\n0\n");
    fprintf(targetFile, "MOV SP, %d\n", currentBindingAddress); // stack base = end of globals
    fprintf(targetFile, "CALL MAIN\n");
    exitProgram(targetFile);
}

void generateFunctionCode(struct Gsymbol* funcSymbol, tnode* body, FILE* targetFile) {
    if (funcSymbol == NULL || body == NULL) {
        LOG_ERROR("Error: Cannot generate code for an invalid function definition\n");
        exit(1);
    }
    int localCount = countLocals();
    emitFunctionLabel(funcSymbol, targetFile);
    emitPrologue(localCount, targetFile);
    codeGen(body, targetFile);
    emitEpilogue(localCount, targetFile);
    freeTree(body);
    LSymbolTableDestroy(Lhead);
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
            LOG_ERROR("Error: Unknown leaf node type %d\n", root->nodetype);
            exit(1);
        }
    }

    // Cases that require special handling: control flow, addresses, calls
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
    case NODE_TYPE_FUNC_CALL:
        return codeGenFuncCall(root, targetFile);
    case NODE_TYPE_NOT:
        return codeGenNot(root, targetFile);
    case NODE_TYPE_TUPLE: 
        return codeGenTupleValue(root, targetFile);
    default:
        break;
    }

    // Statement cases: READ, WRITE, RETURN and ASSIGN
    switch (root->nodetype)
    {
    case NODE_TYPE_READ:
    case NODE_TYPE_WRITE:
    case NODE_TYPE_RETURN:
    case NODE_TYPE_ASSIGN:
        return codeGenStatement(root, targetFile);
    default:
        break;
    }

    return codeGenBinaryOp(root, targetFile);
}
