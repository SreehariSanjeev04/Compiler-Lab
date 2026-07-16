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
    -o stage1

echo "========================================"
echo "Build successful!"
echo "========================================"

./stage1 input.xsm output.xsm


echo "========================================"
echo "Completed!"
echo "========================================"