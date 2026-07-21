#!/bin/bash

set -e

echo "Generating parser..."
bison -d -o build/parser.tab.c src/frontend/parser/parser.y

echo "Generating lexer..."
flex -o build/lex.yy.c src/frontend/lexer/lex.l

echo "Generating label detection lexer..."
flex --prefix=detect -o build/detect.yy.c src/backend/translate/label_detect.l

echo "Generating label translation lexer..."
flex --prefix=translate -o build/translate.yy.c src/backend/translate/label_translate.l

echo "Compiling..."
gcc -g build/parser.tab.c build/lex.yy.c build/detect.yy.c build/translate.yy.c src/frontend/main.c src/frontend/syntaxtree/exprtree.c src/backend/codegen/codegen.c src/backend/translate/translate.c src/backend/translate/labelAddressTable.c -o compiler -Isrc/frontend/syntaxtree -Isrc/frontend/parser -Isrc/backend/codegen -Isrc/backend/translate -Isrc/commons


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