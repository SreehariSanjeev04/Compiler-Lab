#include "logger.h"
#include "codegen.h"
#include <runtime.h>
#include <gsymboltable.h>
#include <lsymboltable.h>
#include "expr_codegen.h"
#include "stmt_codegen.h"

static int countLocals(void) {
    int count = 0;
    struct Lsymbol* current = LSymbolGetHead();
    while (current != NULL) {
        if (current->binding > 0) {
            count++;
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
    case NODE_TYPE_USER_DEREF:
        return codeGenUserType(root, targetFile);
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
