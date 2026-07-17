#!/bin/bash

set -e

echo "Generating parser..."
bison -d -o parser.tab.c src/parser/parser.y

echo "Generating lexer..."
flex -o lex.yy.c src/lexer/lex.l

echo "Compiling..."
gcc -w -g \
    -Isrc/syntaxtree \
    -Isrc/codegen \
    -Isrc/constants \
    parser.tab.c \
    lex.yy.c \
    src/syntaxtree/exprtree.c \
    src/codegen/codegen.c \
    -o stage2

if [ $? -ne 0 ]; then
    echo "========================================"
    echo "Build failed!"
    echo "========================================"
    exit 1
fi

echo "========================================"
echo "Build successful!"
echo "========================================"

./stage2 input.xsm output.xsm

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