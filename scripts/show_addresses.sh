#!/bin/bash

file="$1"

if [ -z "$file" ]; then
    echo "Usage: $0 <program.xsm>"
    exit 1
fi

addr=2048
line=1

while IFS= read -r instr
do
    # Skip blank lines
    [[ -z "$instr" ]] && continue

    # Skip labels (L<number>:)
    if [[ "$instr" =~ ^L[0-9]+:[[:space:]]*$ ]]; then
        echo "           $instr"
        continue
    fi

    printf "%5d  %s\n" "$addr" "$instr"

    if (( line <= 8 )); then
        ((addr++))
    else
        ((addr+=2))
    fi

    ((line++))
done < "$file"