#!/bin/bash

set -e

COUNTEREXAMPLE=false
FIX=false
SHOW_GST=false

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
        -g|--gst)
            SHOW_GST=true
            shift
            ;;
        *)
            break
            ;;
    esac
done

INPUT_FILE="${1:-input.xsm}"
SIM_INPUT="${2:-}"
OUTPUT_FILE="output.xsm"
echo "Input file: $INPUT_FILE"

echo "Generating lexer..."
flex -o build/lex.yy.c src/frontend/lexer/lex.l

echo "Generating parser..."

BISON_ARGS=""
if $COUNTEREXAMPLE; then
    echo "Generating parser with counterexample support..."
    BISON_ARGS="--counterexample"
elif $FIX; then
    echo "Generating parser with fix support..."
    BISON_ARGS="--fix"
fi

bison -d -o build/parser.tab.c src/frontend/parser/parser.y $BISON_ARGS

echo "Generating label detection lexer..."
flex --prefix=detect -o build/detect.yy.c src/backend/translate/label_detect.l

echo "Generating label translation lexer..."
flex --prefix=translate -o build/translate.yy.c src/backend/translate/label_translate.l

echo "Compiling..."
gcc -g build/parser.tab.c build/lex.yy.c build/detect.yy.c build/translate.yy.c src/frontend/main.c src/frontend/syntaxtree/exprtree.c src/backend/codegen/codegen.c src/backend/codegen/expr_codegen.c src/backend/codegen/stmt_codegen.c src/backend/codegen/register_alloc.c src/backend/codegen/runtime.c src/backend/codegen/codegen_utils/codegen_utils.c src/backend/translate/translate.c src/backend/translate/labelAddressTable.c src/frontend/gsymboltable/gsymboltable.c src/frontend/gsymboltable/utils/gsymboltable_utils.c src/frontend/gsymboltable/utils/binding.c src/frontend/gsymboltable/utils/paramlist.c src/frontend/gsymboltable/utils/dimnode.c src/frontend/gsymboltable/utils/flabel.c src/frontend/lsymboltable/lsymboltable.c src/frontend/tupletable/tupletable.c src/frontend/tuplefieldlist/tuplefieldlist.c -o compiler -Isrc/frontend/syntaxtree -Isrc/frontend/parser -Isrc/backend/codegen -Isrc/backend/codegen/codegen_utils -Isrc/backend/translate -Isrc/commons -Isrc/frontend/gsymboltable -Isrc/frontend/gsymboltable/utils -Isrc/frontend/lsymboltable -Isrc/frontend/tupletable -Isrc/frontend/tuplefieldlist -lfl


if [ $? -ne 0 ]; then
    echo "========================================"
    echo "Build failed!"
    echo "========================================"
    exit 1
fi

echo "========================================"
echo "Build successful!"
echo "========================================"

GST_FLAG=""
if $SHOW_GST; then
    GST_FLAG="-g"
    echo "Global symbol table display: enabled"
fi

./compiler $GST_FLAG "$INPUT_FILE" "$OUTPUT_FILE"

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

if [ -n "$SIM_INPUT" ]; then
    echo "Simulator input: $SIM_INPUT"
    ./xsm -l library.lib -e output.xsm < "$SIM_INPUT"
else
    ./xsm -l library.lib -e output.xsm
fi