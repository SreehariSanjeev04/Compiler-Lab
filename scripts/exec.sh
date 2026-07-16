#!/bin/bash

set -e

echo "Generating parser..."
bison -d -o parser.tab.c src/parser/parser.y

echo "Generating lexer..."
flex -o lex.yy.c src/lexer/lex.l

echo "Compiling..."
gcc -w -g \
    -Isrc/syntaxtree \
    -Itest_progs/stage1 \
    parser.tab.c \
    lex.yy.c \
    src/syntaxtree/exprtree.c \
    test_progs/stage1/stage1_ex1.c \
    -o stage1

echo "========================================"
echo "Build successful!"
echo "========================================"

./stage1