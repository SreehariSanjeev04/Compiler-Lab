#!/usr/bin/env bash

# Color configuration
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Check if required tools are in PATH or current directory
XSM_EXEC="./xsm"
if [ ! -f "$XSM_EXEC" ] && ! command -v xsm &> /dev/null; then
    echo -e "${RED}Error: 'xsm' executable not found in current directory or PATH.${NC}"
    exit 1
fi

# 1. Compile the Compiler
echo -e "${BOLD}Compiling compiler...${NC}"

# Run Bison
bison -d -o build/parser.tab.c src/frontend/parser/parser.y
if [ $? -ne 0 ]; then
    echo -e "${RED}Bison generation failed.${NC}"
    exit 1
fi

# Run Flex
flex -o build/lex.yy.c src/frontend/lexer/lex.l
if [ $? -ne 0 ]; then
    echo -e "${RED}Flex generation failed.${NC}"
    exit 1
fi

# Run flex detect for label translation
flex --prefix=detect -o build/detect.yy.c src/backend/translate/label_detect.l

# Run flex translate for label translation
flex --prefix=translate -o build/translate.yy.c src/backend/translate/label_translate.l

# Run GCC
gcc -g build/parser.tab.c build/lex.yy.c build/detect.yy.c build/translate.yy.c src/frontend/main.c src/frontend/syntaxtree/exprtree.c src/backend/codegen/codegen.c src/backend/translate/translate.c src/backend/translate/labelAddressTable.c src/frontend/symboltable/symboltable.c src/frontend/symboltable/utils/utils.c -o compiler -Isrc/frontend/syntaxtree -Isrc/frontend/parser -Isrc/backend/codegen -Isrc/backend/translate -Isrc/commons -Isrc/frontend/symboltable -Isrc/frontend/symboltable/utils

if [ $? -ne 0 ]; then
    echo -e "${RED}GCC compilation failed.${NC}"
    exit 1
fi

echo -e "${GREEN}Compilation successful!${NC}\n"

# 2. Run Testcases
TESTCASE_DIR="testcases" # Or "testcase" - change to match your exact directory name
if [ ! -d "$TESTCASE_DIR" ]; then
    echo -e "${RED}Error: Testcase directory '${TESTCASE_DIR}' not found.${NC}"
    exit 1
fi

# Tracking global pass/fail statistics
TOTAL_PASSED=0
TOTAL_FAILED=0

# Sort the subdirectories naturally (code1, code2, etc.)
subdirs=$(find "$TESTCASE_DIR" -maxdepth 1 -mindepth 1 -type d | sort -V)

for code_dir in $subdirs; do
    code_name=$(basename "$code_dir")
    echo -e "${BOLD}${code_name}${NC}"

    code_file="${code_dir}/code.txt"
    target_xsm="${code_dir}/target.xsm"

    if [ ! -f "$code_file" ]; then
        echo -e "     ${RED}Error: code.txt missing${NC}"
        continue
    fi

    # Run your compiler to generate target.xsm
    ./compiler "$code_file" "$target_xsm" &> /dev/null
    if [ $? -ne 0 ]; then
        echo -e "     ${RED}code generation failed${NC}"
        TOTAL_FAILED=$((TOTAL_FAILED + 1))
        continue
    else
        echo -e "     ${GREEN}code generation successful${NC}"
    fi

    # Look for the internal 'testcases' subdirectory inside codeX/
    inner_tests_dir="${code_dir}/testcases"
    if [ ! -d "$inner_tests_dir" ]; then
        echo -e "     ${RED}Warning: No 'testcases' directory found for ${code_name}${NC}"
        continue
    fi

    # Process each inputX.txt and match it with outputX.txt
    # Sort them naturally (input1.txt, input2.txt...)
    inputs=$(find "$inner_tests_dir" -name "input*.txt" | sort -V)
    
    if [ -z "$inputs" ]; then
        echo -e "     ${RED}No input files found inside testcases/${NC}"
        continue
    fi

    for input_file in $inputs; do
        # Extract the index/number from inputName.txt (e.g., input1.txt -> 1)
        base_input=$(basename "$input_file")
        test_num=$(echo "$base_input" | tr -dc '0-9')
        
        expected_output="${inner_tests_dir}/output${test_num}.txt"
        temp_output="${inner_tests_dir}/temp_output${test_num}.txt"

        if [ ! -f "$expected_output" ]; then
            echo -e "     ${RED}testcase ${test_num} failed (missing expected output file)${NC}"
            TOTAL_FAILED=$((TOTAL_FAILED + 1))
            continue
        fi

        # Execute target code in XSM simulator
        # Passing standard library using library.lib and redirecting stdin/stdout
        if [ -f "library.lib" ]; then
            ./xsm -l library.lib -e "$target_xsm" < "$input_file" > "$temp_output" 2>/dev/null
        else
            ./xsm -e "$target_xsm" < "$input_file" > "$temp_output" 2>/dev/null
        fi

        # Compare outputs while ignoring trailing spaces/newlines
        if diff -Z -B -q "$temp_output" "$expected_output" &> /dev/null; then
            echo -e "     ${GREEN}testcase ${test_num} passed${NC}"
            TOTAL_PASSED=$((TOTAL_PASSED + 1))
        else
            echo -e "     ${RED}testcase ${test_num} failed${NC}"
            TOTAL_FAILED=$((TOTAL_FAILED + 1))
        fi

        # Clean up temporary output files
        if [ -f "$temp_output" ]; then
            rm "$temp_output"
        fi
    done

    # Optional: Clean up generated XSM file after testing
    if [ -f "$target_xsm" ]; then
        rm "$target_xsm"
    fi
done

echo -e "\n${BOLD}Summary: ${TOTAL_PASSED} passed, ${TOTAL_FAILED} failed${NC}"

if [ $TOTAL_FAILED -gt 0 ]; then
    exit 1
else
    exit 0
fi