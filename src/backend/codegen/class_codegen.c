#include "class_codegen.h"
#include "codegen.h"
#include "register_alloc.h"
#include "expr_codegen.h"
#include "logger.h"
#include "classtable.h"

#include <string.h>

/* Emits the dereferenced address of into the target register*/
void emitClassFieldAccessAddress(
    int targetReg,
    tnode *root,
    FILE *targetFile
) {
    if (root == NULL) {
        fprintf(stderr, "NULL AST node\n");
        exit(EXIT_FAILURE);
    }

    if (root->nodetype == NODE_TYPE_ID) {

        if (root->Lentry != NULL) {
            fprintf(targetFile, "MOV R%d, BP\n", targetReg);

            int offset = root->Lentry->binding;

            if (offset < 0)
                fprintf(targetFile, "SUB R%d, %d\n",
                        targetReg, -offset);
            else
                fprintf(targetFile, "ADD R%d, %d\n",
                        targetReg, offset);

        } else {
            fprintf(targetFile, "MOV R%d, %d\n",
                    targetReg, root->Gentry->binding);
        }

        return;
    } else if (root->nodetype == NODE_TYPE_SELF) {
        // self node, should get the address of the object
        
        struct Lsymbol* selfEntry = LLookup("self");
        if (selfEntry == NULL) {
            LOG_ERROR("'self' can be only used within functions\n");
            exit(1);
        }
        int binding = selfEntry->binding;
        fprintf(targetFile, "MOV R%d, BP\n", targetReg);
        if (binding < 0) {
            fprintf(targetFile, "SUB R%d, %d\n",
                    targetReg, -binding);
        } else {
            fprintf(targetFile, "ADD R%d, %d\n",
                    targetReg, binding);
        } // address of BP-k
        return; 
    }

    emitClassFieldAccessAddress(targetReg, root->left, targetFile);

    fprintf(targetFile, "MOV R%d, [R%d]\n",
            targetReg, targetReg);

    fprintf(targetFile, "ADD R%d, %d\n",
            targetReg, root->val);
}


int codeGenClassFieldAccess(tnode *root, FILE* targetFile) {
    if (root == NULL) {
        LOG_ERROR("The 'root' is NULL\n");
        exit(1);
    }
    int addressReg = getReg();
    emitClassFieldAccessAddress(addressReg, root, targetFile);
    fprintf(targetFile, "MOV R%d, [R%d]\n", addressReg, addressReg);
    return addressReg;
}

static int pushMethodArguments(tnode *args, FILE *targetFile) {
    if (args == NULL) return 0;
    if (args->nodetype == NODE_TYPE_ARG) {
        int count = pushMethodArguments(args->right, targetFile);
        return count + pushMethodArguments(args->left, targetFile);
    }

    int reg = codeGen(args, targetFile);
    fprintf(targetFile, "PUSH R%d\n", reg);
    freeReg();
    return 1;
}

int codeGenClassMethodAccess(tnode *root, FILE *targetFile) {
    if (root == NULL || root->nodetype != NODE_TYPE_CLASS_METHOD_ACCESS ||
        root->left == NULL || root->left->cPtr == NULL) {
        LOG_ERROR("Invalid class method call.\n");
        exit(1);
    }

    MemberFuncList *method = CMemLookup(root->left->cPtr, root->varname);
    if (method == NULL) {
        LOG_ERROR("Method '%s' was not found.\n", root->varname);
        exit(1);
    }

    int savedCount = getRegCount();
    saveRegisters(targetFile);

    int receiverReg = getReg();
    emitClassFieldAccessAddress(receiverReg, root->left, targetFile);
    fprintf(targetFile, "MOV R%d, [R%d]\n", receiverReg, receiverReg);
    fprintf(targetFile, "PUSH R%d\n", receiverReg);
    freeReg();

    int argWords = pushMethodArguments(root->right, targetFile);

    int slotReg = getReg();
    fprintf(targetFile, "MOV R%d, 0\nPUSH R%d\n", slotReg, slotReg);
    freeReg();

    fprintf(targetFile, "CALL F%d\n", method->Flabel);

    int resultReg = getReg();
    fprintf(targetFile, "POP R%d\n", resultReg);

    int discardReg = resultReg == MAX_REGISTERS - 1 ? MAX_REGISTERS - 2 : MAX_REGISTERS - 1;
    for (int i = 0; i < argWords + 1; i++) {
        fprintf(targetFile, "POP R%d\n", discardReg);
    }
    for (int i = savedCount - 1; i >= 0; i--) {
        fprintf(targetFile, "POP R%d\n", i);
    }
    return resultReg;
}

int codeGenClassDelete(tnode *root, FILE *targetFile) {
    if (root == NULL || root->nodetype != NODE_TYPE_DELETE || root->Gentry == NULL) {
        LOG_ERROR("Invalid class delete node.\n");
        exit(1);
    }

    int reg = getReg();
    fprintf(targetFile, "MOV R%d, %d\n", reg, root->Gentry->binding);
    fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
    codeGenLibraryCall("Free", reg, 0, targetFile);

    int doneLabel = generateLabel();
    fprintf(targetFile, "JNZ R%d, L%d\n", reg, doneLabel);
    fprintf(targetFile, "MOV R%d, %d\n", reg, root->Gentry->binding);
    fprintf(targetFile, "MOV [R%d], 0\n", reg);
    fprintf(targetFile, "L%d:\n", doneLabel);

    freeReg();
    return -1;
}

int codeGenSelf(FILE* targetFile) {
    struct Lsymbol *selfEntry = LLookup("self");
    if (selfEntry == NULL) {
        LOG_ERROR("'self' can be only used within classes\n");
        exit(1);
    }
    int reg = getReg();
    fprintf(targetFile, "MOV R%d, BP\n", reg);
    fprintf(targetFile, "SUB R%d, %d\n", reg, -selfEntry->binding);
    fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
    return reg;
}
