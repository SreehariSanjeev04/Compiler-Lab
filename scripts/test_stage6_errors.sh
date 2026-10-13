#!/usr/bin/env bash
set -euo pipefail

tmp_dir=$(mktemp -d)
trap 'rm -rf "$tmp_dir"' EXIT

passed=0
for source in testcases/invalid_stage6/*.txt; do
    name=${source##*/}
    name=${name%.txt}
    if ./compiler "$source" "$tmp_dir/$name.xsm" >"$tmp_dir/$name.log" 2>&1; then
        echo "$name: unexpectedly compiled"
        exit 1
    fi
    if ! grep -F -q -f "testcases/invalid_stage6/$name.expected" "$tmp_dir/$name.log"; then
        echo "$name: expected diagnostic missing"
        cat "$tmp_dir/$name.log"
        exit 1
    fi
    echo "$name: rejected as expected"
    passed=$((passed + 1))
done

echo "$passed Stage 6 invalid type cases passed"
