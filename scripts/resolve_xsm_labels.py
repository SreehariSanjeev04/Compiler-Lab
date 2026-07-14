#!/usr/bin/env python3

import re
import sys
from pathlib import Path

PROGRAM_START = 2048
HEADER_WORDS = 8
HEADER_WORD_SIZE = 1
INSTRUCTION_SIZE = 2
LABEL_DEF_RE = re.compile(r'^\s*([A-Za-z_][A-Za-z0-9_]*)\s*:\s*(?:;.*)?$')
WORD_REF_RE = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*)\b')


def strip_comment(line: str) -> str:
    if '//' in line:
        return line.split('//', 1)[0]
    return line


def main() -> int:
    if len(sys.argv) != 2:
        print(f'Usage: {sys.argv[0]} <input.xsm>', file=sys.stderr)
        return 1

    source_path = Path(sys.argv[1])
    if not source_path.is_file():
        print(f'Error: file not found: {source_path}', file=sys.stderr)
        return 1

    raw_lines = source_path.read_text().splitlines()

    labels = {}
    current_address = PROGRAM_START
    emitted_lines = 0

    for line in raw_lines:
        code = strip_comment(line).strip()
        if not code:
            continue

        match = LABEL_DEF_RE.match(code)
        if match:
            labels[match.group(1)] = current_address
            continue

        if emitted_lines < HEADER_WORDS:
            current_address += HEADER_WORD_SIZE
        else:
            current_address += INSTRUCTION_SIZE
        emitted_lines += 1

    resolved_lines = []
    for line in raw_lines:
        original_line = line
        comment = ''
        if '//' in original_line:
            original_line, comment = original_line.split('//', 1)
            comment = '//' + comment

        stripped = original_line.strip()
        if not stripped:
            continue

        if LABEL_DEF_RE.match(stripped):
            continue

        def replace_token(match: re.Match[str]) -> str:
            token = match.group(1)
            return str(labels[token]) if token in labels else token

        replaced = WORD_REF_RE.sub(replace_token, original_line)
        resolved_lines.append(replaced.rstrip() + ((' ' + comment) if comment else ''))

    sys.stdout.write('\n'.join(resolved_lines))
    if resolved_lines:
        sys.stdout.write('\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
