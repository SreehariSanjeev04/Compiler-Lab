# XSM & eXpl — Compiler & Virtual Machine Lab

A university compiler-design lab project comprising a full compiler toolchain and a custom virtual machine. An **eXpl** (high-level) program is compiled to **XSM** assembly and executed on the **XSM simulator**.

## Components

| Component | Description |
|---|---|
| **eXpl Compiler** | Compiles a C-like language (eXpl) into XSM assembly. Lexer (Flex), Parser (Bison), AST, code generator, and a label translator. |
| **XSM Simulator** | Virtual CPU with 35 opcodes, 33 registers, paged memory (64 KB RAM), disk I/O, privilege modes, interrupts, and an interactive debugger. |
| **SPL Compiler** | Compiles the System Programmer's Language (SPL) — a low-level language for writing OS code (interrupt handlers, exception routines, etc.) — into XSM assembly. |
| **XFS Interface** | A CLI tool with readline support for managing files on a simulated XFS disk (`disk.xfs`). Commands: `fdisk`, `load`, `rm`, `ls`, `cat`, `copy`, `dump`, `export`. |

## Directory Layout

```
xsm_expl/
├── src/               # eXpl compiler source (C + Flex + Bison)
│   ├── frontend/      #   Lexer, Parser, Symbol Table, AST
│   └── backend/       #   Code generator, Label translator
├── xsm_dev/           # XSM machine simulator (C + Flex)
├── spl/               # SPL compiler (C + Flex + Bison)
├── xfs-interface/     # XFS disk filesystem interface (C)
├── scripts/           # Build/test/utility scripts
├── testcases/         # 25 eXpl test programs
└── test_progs/        # Stage-wise test programs
```

## Build

**Prerequisites:** `gcc`, `flex`, `bison`, `libreadline-dev`

```sh
# Build everything (XSM simulator, SPL compiler, XFS interface)
make

# Or build individual components
make -C xsm_dev
make -C spl
make -C xfs-interface

# Build and run the eXpl compiler
cd scripts && ./exec.sh
```

## Usage

### eXpl → XSM compilation

```sh
./compiler input.expl output.xsm
```

### Run on the XSM simulator

```sh
./xsm -l library.lib -e program.xsm
```

Use `--debug` for the interactive debugger (step, continue, watchpoints, register/memory display).

### SPL compilation

```sh
./spl/spl <source.spl>
```

### XFS filesystem

```sh
./xfs-interface/xfs-interface
```

## The eXpl Language

A simple C-like language supporting:

- `int` and `str` types, arrays
- `read` / `write` statements
- Arithmetic and relational expressions
- `if` / `if-else`, `while`, `do-while`, `repeat-until`
- `break`, `continue`, `breakpoint`

### Example

```
begin
decl
    int a, b;
    int arr[10];
    str mesg;
enddecl

read(a);
b = a + 2 * 5;
write(b);

if (a > b) then
    write(a);
endif;

while (a != 0) do
    a = a - 1;
endwhile;

mesg = "hello";
write(mesg);
end;
```

## Test

```sh
./scripts/run_tests.sh
```

Runs all 25 test cases (`testcases/code1` through `code25`), comparing output against expected results.
