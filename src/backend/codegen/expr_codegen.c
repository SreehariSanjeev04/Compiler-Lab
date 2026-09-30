#include "logger.h"
#include "codegen.h"
#include "expr_codegen.h"
#include <codegen_utils.h>
#include <gsymboltable.h>
#include <lsymboltable.h>
#include <gsymboltable_utils.h>
#include <register_alloc.h>
#include <binding.h>
#include <tupletable.h>

int codeGenArrayAddress(tnode *root, FILE *targetFile)
{
    tnode *spine[MAX_ARRAY_DIMENSION + 1];
    int depth = 0;

    tnode *current = root;
    while (current->nodetype == NODE_TYPE_ARRAY)
    {
        if (depth > MAX_ARRAY_DIMENSION)
        {
            LOG_ERROR("Error: Array nesting exceeds the maximum supported depth\n");
            exit(1);
        }
        spine[depth++] = current;
        current = current->left;
    }
    if (current->nodetype != NODE_TYPE_ID)
    {
        LOG_ERROR("Error: Array node must have an ID as its leftmost child\n");
        exit(1);
    }
    struct Gsymbol *symbol = GLookup(current->varname);
    if (symbol == NULL)
    {
        LOG_ERROR("Error: Variable '%s' not defined\n", current->varname);
        exit(1);
    }
    if (depth > symbol->dimensions)
    {
        LOG_ERROR("Error: Variable '%s' expects at most %d subscript(s), got %d\n",
                current->varname, symbol->dimensions, depth);
        exit(1);
    }
    int bindingAddress = returnStaticBindAddress(current->varname);
    int *strideArray = createStrideArray(symbol);

    // Start the total offset with the binding address of the array
    int totalOffsetReg = getReg();
    fprintf(targetFile, "MOV R%d, %d\n", totalOffsetReg, bindingAddress);

    for (int dim = 0; dim < depth; dim++)
    {
        // spine is ordered outermost-node-first, but each node's right child
        // holds the NEXT (inner) subscript -- so dimension `dim` (0 =
        // outermost) is found at spine[depth-1-dim]->right.
        int computeReg = getReg();
        int indexReg = codeGen(spine[depth - 1 - dim]->right, targetFile);
        fprintf(targetFile, "MOV R%d, %d\n", computeReg, strideArray[dim]);
        fprintf(targetFile, "MUL R%d, R%d\n", computeReg, indexReg);
        fprintf(targetFile, "ADD R%d, R%d\n", totalOffsetReg, computeReg);
        freeReg(); // free indexReg (allocated last)
        freeReg(); // free computeReg
    }
    free(strideArray);
    return totalOffsetReg;
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
    if (root->nodetype == NODE_TYPE_ID && root->Gentry != NULL && root->Gentry->dimensions > 0)
    {
        int reg = getReg();
        fprintf(targetFile, "MOV R%d, %d\n", reg, root->Gentry->binding);
        return reg;
    }
    return codeGen(root, targetFile);
}

int codeGenTupleAddress(tnode *root, FILE *targetFile)
{
    if (root == NULL || root->nodetype != NODE_TYPE_TUPLE)
    {
        LOG_ERROR("Error: Tuple field access node expected\n");
        exit(1);
    }

    tnode *base = root->left;
    struct TupleTable *entry = NULL;

    if (base->nodetype == NODE_TYPE_ID)
    {
        if (base->Gentry != NULL) entry = base->Gentry->tupleEntry;
        else if (base->Lentry != NULL) entry = base->Lentry->tupleEntry;
        else {
            LOG_ERROR("Error: '%s' is not a tuple variable\n", base->varname);
            exit(1);
        }
    }
    else if (base->nodetype == NODE_TYPE_DEREF)
    {
        tnode *inner = base->left;
        if (inner == NULL || inner->nodetype != NODE_TYPE_ID ||
            inner->Gentry == NULL || inner->Gentry->tupleEntry == NULL)
        {
            LOG_ERROR("Error: Invalid pointer-to-tuple field access\n");
            exit(1);
        }
        entry = inner->Gentry->tupleEntry;
    }
    else
    {
        LOG_ERROR("Error: Only tuple variables or dereferenced tuple pointers support field access\n");
        exit(1);
    }

    if (entry == NULL)
    {
        LOG_ERROR("Error: '%s' is not a tuple variable\n", base->varname);
        exit(1);
    }

    struct TupleFieldList *field = TupleFieldListLookup(entry->name, root->varname);
    if (field == NULL)
    {
        LOG_ERROR("Error: Tuple type '%s' has no field named '%s'\n", entry->name, root->varname);
        exit(1);
    }

    int baseReg = getReg();
    if (base->nodetype == NODE_TYPE_DEREF)
    {
        // Address of (*sptr) is the pointer value stored in sptr.
        emitVarAddressInto(baseReg, base->left->varname, targetFile);
        fprintf(targetFile, "MOV R%d, [R%d]\n", baseReg, baseReg);
    }
    else if (base->pointerLevel == 0)
    {
        emitVarAddressInto(baseReg, base->varname, targetFile);
    }
    else
    {
        // sptr.field on a pointer-to-tuple: dereference the pointer first.
        emitVarAddressInto(baseReg, base->varname, targetFile);
        fprintf(targetFile, "MOV R%d, [R%d]\n", baseReg, baseReg);
    }

    if (field->offset > 0)
    {
        fprintf(targetFile, "ADD R%d, %d\n", baseReg, field->offset);
    }

    return baseReg;
}

int codeGenTupleValue(tnode *root, FILE *targetFile)
{
    int valueReg = getReg();
    int addrReg = codeGenTupleAddress(root, targetFile);
    fprintf(targetFile, "MOV R%d, [R%d]\n", valueReg, addrReg);
    freeReg(); // frees addrReg (most recently allocated)
    return valueReg;
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
        if (args[i]->type == TYPE_TUPLE && args[i]->pointerLevel == 0)
        {
            struct TupleTable *tupleType = args[i]->Gentry ? args[i]->Gentry->tupleEntry :
                                           args[i]->Lentry ? args[i]->Lentry->tupleEntry : NULL;
            if (args[i]->nodetype != NODE_TYPE_ID || tupleType == NULL)
            {
                LOG_ERROR("Error: Tuple argument must be a tuple variable\n");
                exit(1);
            }
            int addressReg = getReg();
            emitVarAddressInto(addressReg, args[i]->varname, targetFile);
            int valueReg = getReg();
            for (int offset = 0; offset < tupleType->size; offset++)
            {
                fprintf(targetFile, "MOV R%d, [R%d]\n", valueReg, addressReg);
                fprintf(targetFile, "PUSH R%d\n", valueReg);
                if (offset + 1 < tupleType->size)
                    fprintf(targetFile, "ADD R%d, 1\n", addressReg);
            }
            freeReg(); // valueReg
            freeReg(); // addressReg
            words += tupleType->size;
        }
        else
        {
            int argReg = codeGen(args[i], targetFile);
            fprintf(targetFile, "PUSH R%d\n", argReg);
            freeReg();
            words++;
        }
    }
    return words;
}

int codeGenFuncCall(tnode *root, FILE *targetFile)
{
    struct Gsymbol *funcSymbol = root->Gentry;
    if (funcSymbol->type == TYPE_TUPLE && funcSymbol->pointerLevel == 0)
    {
        LOG_ERROR("Error: Tuple-valued call requires a tuple assignment destination\n");
        exit(1);
    }
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

void codeGenTupleCallInto(tnode *call, tnode *destination, FILE *targetFile)
{
    struct Gsymbol *function = call->Gentry;
    struct TupleTable *tupleType = function ? function->tupleEntry : NULL;
    if (tupleType == NULL || tupleType->size <= 0)
    {
        LOG_ERROR("Error: Tuple-valued function has no tuple type\n");
        exit(1);
    }

    int savedCount = getRegCount();
    saveRegisters(targetFile);

    int argc = countCallArgs(call->left);
    tnode **args = NULL;
    if (argc > 0)
    {
        args = malloc((size_t)argc * sizeof(*args));
        if (args == NULL)
        {
            LOG_ERROR("Error: Memory allocation failed for call arguments\n");
            exit(1);
        }
        int index = 0;
        flattenCallArgs(call->left, args, &index);
    }
    int argWords = pushCallArguments(args, argc, targetFile);

    int slotReg = getReg();
    fprintf(targetFile, "MOV R%d, 0\n", slotReg);
    for (int i = 0; i < tupleType->size; i++)
        fprintf(targetFile, "PUSH R%d\n", slotReg);
    freeReg();
    fprintf(targetFile, "CALL F%d\n", function->flabel);

    int dstReg = getReg();
    emitVarAddressInto(dstReg, destination->varname, targetFile);
    int valueReg = getReg();
    for (int offset = 0; offset < tupleType->size; offset++)
    {
        fprintf(targetFile, "POP R%d\n", valueReg);
        fprintf(targetFile, "MOV [R%d], R%d\n", dstReg, valueReg);
        if (offset + 1 < tupleType->size)
            fprintf(targetFile, "ADD R%d, 1\n", dstReg);
    }
    for (int i = 0; i < argWords; i++)
        fprintf(targetFile, "POP R%d\n", valueReg);
    freeReg(); // valueReg
    freeReg(); // dstReg
    restoreRegisters(targetFile);
    free(args);
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
        // Load the variable's value: locals/params via [BP+offset], globals via
        // their static binding.
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
        // Used as a value in an expression: compute the element address, then dereference.
        int reg = codeGenArrayAddress(root, targetFile);
        fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
        return reg;
    }
    case NODE_TYPE_ADDRESS:
    {
        if (root->left->nodetype != NODE_TYPE_ID &&
            root->left->nodetype != NODE_TYPE_ARRAY &&
            root->left->nodetype != NODE_TYPE_TUPLE)
        {
            LOG_ERROR("Error: Address node must have an ID, array or tuple-access node as its left child\n");
            exit(1);
        }
        if (root->left->nodetype == NODE_TYPE_ARRAY)
            return codeGenArrayAddress(root->left, targetFile); // computes the base address + offset
        if (root->left->nodetype == NODE_TYPE_TUPLE)
            return codeGenTupleAddress(root->left, targetFile);

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
            (root->left->nodetype == NODE_TYPE_ID && root->left->Gentry != NULL && root->left->Gentry->dimensions > 0))
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
