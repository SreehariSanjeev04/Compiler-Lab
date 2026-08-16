#!/bin/bash

set -e

COUNTEREXAMPLE=false
FIX=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--counterexample)
            COUNTEREXAMPLE=true
            shift
            ;;
        -f|--fix)
            FIX=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac 
done

echo "Generating parser..."

if $COUNTEREXAMPLE; then
    echo "Generating parser with counterexample support..."
    bison -d -o build/parser.tab.c src/frontend/parser/parser.y --counterexample
else
    bison -d -o build/parser.tab.c src/frontend/parser/parser.y
fi

if $FIX; then
    echo "Generating parser with fix support..."
    bison -d -o build/parser.tab.c src/frontend/parser/parser.y --fix
else
    bison -d -o build/parser.tab.c src/frontend/parser/parser.y
fi

echo "Generating label detection lexer..."
flex --prefix=detect -o build/detect.yy.c src/backend/translate/label_detect.l

echo "Generating label translation lexer..."
flex --prefix=translate -o build/translate.yy.c src/backend/translate/label_translate.l

echo "Compiling..."
gcc -g build/parser.tab.c build/lex.yy.c build/detect.yy.c build/translate.yy.c src/frontend/main.c src/frontend/syntaxtree/exprtree.c src/backend/codegen/codegen.c src/backend/codegen/expr_codegen.c src/backend/codegen/stmt_codegen.c src/backend/codegen/register_alloc.c src/backend/codegen/runtime.c src/backend/codegen/codegen_utils/codegen_utils.c src/backend/translate/translate.c src/backend/translate/labelAddressTable.c src/frontend/symboltable/symboltable.c src/frontend/symboltable/utils/symboltable_utils.c src/frontend/symboltable/utils/binding.c -o compiler -Isrc/frontend/syntaxtree -Isrc/frontend/parser -Isrc/backend/codegen -Isrc/backend/codegen/codegen_utils -Isrc/backend/translate -Isrc/commons -Isrc/frontend/symboltable -Isrc/frontend/symboltable/utils -lfl


if [ $? -ne 0 ]; then
    echo "========================================"
    echo "Build failed!"
    echo "========================================"
    exit 1
fi

echo "========================================"
echo "Build successful!"
echo "========================================"

./compiler input.xsm output.xsm

status=$?
if [ $status -ne 0 ]; then

    echo "========================================"
    echo "Execution failed with status $status"
    echo "========================================"
    exit $status

fi


echo "========================================"
echo "Completed!"
echo "========================================"


echo "========================================"
echo "Executing the generated code..."
echo "========================================"

./xsm -l library.lib -e output.xsm