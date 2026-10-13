#include "logger.h"
#include "codegen.h"
#include "expr_codegen.h"
#include <codegen_utils.h>
#include <gsymboltable.h>
#include <lsymboltable.h>
#include <gsymboltable_utils.h>
#include <register_alloc.h>
#include <fieldlist.h>
#include <binding.h>

int codeGenArrayAddress(tnode *root, FILE *targetFile)
{
    if (root == NULL || root->nodetype != NODE_TYPE_ARRAY ||
        root->left == NULL || root->left->nodetype != NODE_TYPE_ID)
    {
        LOG_ERROR("Error: Array access requires one index on a global array\n");
        exit(1);
    }
    struct Gsymbol *symbol = root->left->Gentry;
    if (symbol == NULL || symbol->arrayLength == 0)
    {
        LOG_ERROR("Error: '%s' is not an array\n", root->left->varname);
        exit(1);
    }
    int addressReg = getReg();
    fprintf(targetFile, "MOV R%d, %d\n", addressReg, symbol->binding);
    int indexReg = codeGen(root->right, targetFile);
    fprintf(targetFile, "ADD R%d, R%d\n", addressReg, indexReg);
    freeReg();
    return addressReg;
}

 
void emitUserTypeAddressInto(int reg, tnode* root, FILE *targetFile) {
    if (root == NULL || root->left == NULL || root->right == NULL) {
        LOG_ERROR("Error: Invalid AST node in user type address calculation.\n");
        return;
    }

    if (root->left->nodetype == NODE_TYPE_ID) {
        Gsymbol *gentry = GLookup(root->left->varname);
        if (gentry == NULL) {
            LOG_ERROR("Undeclared variable: %s\n", root->left->varname);
            return;
        }

        fprintf(targetFile, "MOV R%d, %d\n", reg, gentry->binding);

        if (root->right->nodetype != NODE_TYPE_ID) {
            LOG_ERROR("Invalid field access syntax.\n");
            return;
        }

        struct FieldList* fentry = FLookup(root->left->type, root->right->varname);
        if (fentry == NULL) {
            LOG_ERROR("Field '%s' not found in type '%s'\n", root->right->varname, gentry->type->name);
            return;
        }

        fprintf(targetFile, "ADD R%d, %d\n", reg, fentry->fieldIndex);
        return;
    }

    emitUserTypeAddressInto(reg, root->left, targetFile);

    struct FieldList* fentry = FLookup(root->left->type, root->right->varname);
    if (fentry == NULL) {
        LOG_ERROR("Field '%s' not found in type '%s'\n", root->right->varname, root->left->right->varname);
        return;
    }

    fprintf(targetFile, "ADD R%d, %d\n", reg, fentry->fieldIndex);
}

void emitVarAddressInto(int reg, const char *varname, FILE *targetFile)
{
    struct Lsymbol *lentry = LLookup((char *)varname);
    if (lentry != NULL)
    {
        fprintf(targetFile, "MOV R%d, BP\n", reg);
        fprintf(targetFile, "ADD R%d, %d\n", reg, lentry->binding);
        return;
    }
    struct Gsymbol *gentry = GLookup(varname);
    if (gentry == NULL)
    {
        LOG_ERROR("Error: Variable '%s' not defined\n", varname);
        exit(1);
    }
    fprintf(targetFile, "MOV R%d, %d\n", reg, gentry->binding);
}

int codeGenAddressOperand(tnode *root, FILE *targetFile)
{
    if (root->nodetype == NODE_TYPE_ARRAY)
    {
        return codeGenArrayAddress(root, targetFile);
    }
    if (root->nodetype == NODE_TYPE_ID && root->Gentry != NULL && root->Gentry->arrayLength > 0)
    {
        int reg = getReg();
        fprintf(targetFile, "MOV R%d, %d\n", reg, root->Gentry->binding);
        return reg;
    }
    return codeGen(root, targetFile);
}

static int countCallArgs(tnode *argsNode)
{
    if (argsNode == NULL) return 0;
    if (argsNode->nodetype != NODE_TYPE_ARG) return 1;
    return countCallArgs(argsNode->left) + 1;
}

static void flattenCallArgs(tnode *argsNode, tnode **argArray, int *index)
{
    if (argsNode == NULL) return;
    if (argsNode->nodetype != NODE_TYPE_ARG)
    {
        argArray[(*index)++] = argsNode;
        return;
    }
    flattenCallArgs(argsNode->left, argArray, index);
    argArray[(*index)++] = argsNode->right;
}

static int pushCallArguments(tnode **args, int argc, FILE *targetFile)
{
    int words = 0;
    for (int i = argc - 1; i >= 0; i--)
    {
        int argReg = codeGen(args[i], targetFile);
        fprintf(targetFile, "PUSH R%d\n", argReg);
        freeReg();
        words++;
    }
    return words;
}

int codeGenFuncCall(tnode *root, FILE *targetFile)
{
    struct Gsymbol *funcSymbol = root->Gentry;
    int savedCount = getRegCount();
    saveRegisters(targetFile);

    int argc = countCallArgs(root->left);
    tnode **args = NULL;
    if (argc > 0)
    {
        args = (tnode **)malloc(argc * sizeof(tnode *));
        if (args == NULL)
        {
            LOG_ERROR("Error: Memory allocation failed for call arguments\n");
            exit(1);
        }
        int index = 0;
        flattenCallArgs(root->left, args, &index);
    }

    int argWords = pushCallArguments(args, argc, targetFile);

    int slotReg = getReg();
    fprintf(targetFile, "MOV R%d, 0\n", slotReg);
    fprintf(targetFile, "PUSH R%d\n", slotReg);
    freeReg();

    fprintf(targetFile, "CALL F%d\n", funcSymbol->flabel);

    int resultReg = getReg(); 
    fprintf(targetFile, "POP R%d\n", resultReg);

    for (int i = 0; i < argWords; i++)
    {
        int discardReg = getReg();
        fprintf(targetFile, "POP R%d\n", discardReg);
        freeReg();
    }

    for (int i = savedCount - 1; i >= 0; i--)
    {
        fprintf(targetFile, "POP R%d\n", i);
    }

    free(args);
    return resultReg;
}

int codeGenNot(tnode *root, FILE *targetFile)
{
    int operandReg = codeGen(root->left, targetFile);
    int zeroReg = getReg();
    fprintf(targetFile, "MOV R%d, 0\n", zeroReg);
    fprintf(targetFile, "EQ R%d, R%d\n", operandReg, zeroReg);
    freeReg();
    return operandReg;
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
        int valueReg = getReg();
        int addressReg = getReg();
        emitVarAddressInto(addressReg, root->varname, targetFile);
        fprintf(targetFile, "MOV R%d, [R%d]\n", valueReg, addressReg);
        freeReg(); // free addressReg
        return valueReg;
    }
    default:
        LOG_ERROR("Error: Unknown leaf node type %d\n", root->nodetype);
        exit(1);
    }
}

int codeGenAddressExpr(tnode *root, FILE *targetFile)
{
    switch (root->nodetype)
    {
    case NODE_TYPE_ARRAY:
    {
        int reg = codeGenArrayAddress(root, targetFile);
        fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
        return reg;
    }
    case NODE_TYPE_ADDRESS:
    {
        if (root->left->nodetype != NODE_TYPE_ID &&
            root->left->nodetype != NODE_TYPE_ARRAY)
        {
            LOG_ERROR("Error: Address node must have an ID or array node as its left child\n");
            exit(1);
        }
        if (root->left->nodetype == NODE_TYPE_ARRAY)
            return codeGenArrayAddress(root->left, targetFile); // computes the base address + offset

        int reg = getReg();
        emitVarAddressInto(reg, root->left->varname, targetFile); // load the address into the register
        return reg;
    }
    case NODE_TYPE_DEREF:
    {
        if (effectivePointerLevel(root->left) <= 0)
        {
            LOG_ERROR("Error: Cannot dereference a non-pointer expression\n");
            exit(1);
        }
        if (root->left->nodetype == NODE_TYPE_ARRAY ||
            (root->left->nodetype == NODE_TYPE_ID && root->left->Gentry != NULL && root->left->Gentry->arrayLength > 0))
        {

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
        LOG_ERROR("Error: Unknown node type %d\n", root->nodetype);
        exit(1);
    }
}

int codeGenBinaryOp(tnode *root, FILE *targetFile)
{
    // below this is arithmetic and relational operations, which are binary operations
    if (!isArithmeticCompatible(root->left, root->right, root->nodetype))
    {
        LOG_ERROR("Error: Type Mismatch in arithmetic or relational operation\n");
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
    case NODE_TYPE_MOD:
    {
        fprintf(targetFile, "MOD R%d, R%d\n", leftReg, rightReg);
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
    case NODE_TYPE_NE:
        fprintf(targetFile, "NE R%d, R%d\n", leftReg, rightReg);
        break;
    case NODE_TYPE_AND:
    {
        // Logical AND on normalized booleans: (l != 0) MUL (r != 0)
        int zeroReg = getReg();
        fprintf(targetFile, "MOV R%d, 0\n", zeroReg);
        fprintf(targetFile, "NE R%d, R%d\n", leftReg, zeroReg);
        fprintf(targetFile, "NE R%d, R%d\n", rightReg, zeroReg);
        freeReg();
        fprintf(targetFile, "MUL R%d, R%d\n", leftReg, rightReg);
        break;
    }
    case NODE_TYPE_OR:
    {
        // Logical OR on normalized booleans: ((l != 0) ADD (r != 0)) GT 0
        int zeroReg = getReg();
        fprintf(targetFile, "MOV R%d, 0\n", zeroReg);
        fprintf(targetFile, "NE R%d, R%d\n", leftReg, zeroReg);
        fprintf(targetFile, "NE R%d, R%d\n", rightReg, zeroReg);
        fprintf(targetFile, "ADD R%d, R%d\n", leftReg, rightReg);
        fprintf(targetFile, "GT R%d, R%d\n", leftReg, zeroReg);
        freeReg();
        break;
    }
    default:
        LOG_ERROR("Error: Unknown node type %d\n", root->nodetype);
        exit(1);
    }

    freeReg();
    return leftReg;
}
