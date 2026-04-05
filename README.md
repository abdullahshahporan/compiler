# S++ Compiler (SPP) — Bangla-Keyword Programming Language

**Course:** CSE 3212 — Compiler Design Lab

## Overview

S++ is a custom programming language that uses **Bangla keywords** instead of English.
The compiler reads S++ source code, builds an AST, performs semantic analysis,
interprets the program, generates three-address intermediate code, optimizes it,
and translates the AST to equivalent C code.

## Language Keywords

| S++ Keyword | English Meaning | Usage |
|---|---|---|
| `purno` | int | Integer type |
| `dosomik` | float | Floating-point type |
| `chinho` | char | Character type |
| `kisu_na` | void | Void type |
| `shuru` / `shesh` | begin / end | Block delimiters |
| `jodi` / `nahole` | if / else | Conditional |
| `ghurao` | while | While loop |
| `barbar` | for | For loop |
| `bondho` | break | Break statement |
| `chaliyejao` | continue | Continue statement |
| `ferot` | return | Return statement |
| `banaw` | make | Void function declaration |
| `dekhao` | print | Output |
| `neo` | input | Input |
| `jog` / `biyog` / `gun` / `vag` | +  /  -  /  *  /  / | Arithmetic operators |
| `root` | sqrt | Square root |
| `shotto` / `mittha` | true / false | Boolean literals |

## Compiler Architecture (6 Phases)

```
Source Code (.txt)
    │
    ▼
┌──────────┐     ┌──────────┐
│  Lexer   │────▶│  Parser  │──▶ AST (Abstract Syntax Tree)
│ (Flex)   │     │ (Bison)  │
└──────────┘     └──────────┘
                       │
         ┌─────────────┼─────────────┐
         ▼             ▼             ▼
   ┌──────────┐  ┌───────────┐  ┌──────────┐
   │ Semantic │  │Interpreter│  │   TAC    │
   │ Analysis │  │(Tree-walk)│  │Generator │
   └──────────┘  └───────────┘  └──────────┘
                                     │
                               ┌─────┴─────┐
                               ▼           ▼
                         ┌──────────┐ ┌──────────┐
                         │Optimizer │ │ C Code   │
                         │(Const.  │ │Generator │
                         │ Folding)│ └──────────┘
                         └──────────┘
```

| Phase | Module | Description |
|---|---|---|
| 1 | `spp_lexer.l` → Flex | Tokenizes Bangla keywords, literals, operators |
| 2 | `spp_parser.y` → Bison | Builds AST from grammar rules |
| 3 | `src/semantic.c` | Type checking, undeclared vars, arity checks |
| 4 | `src/interpreter.c` | Tree-walk execution with runtime error detection |
| 5 | `src/tac.c` + `src/optimizer.c` | Three-address code + constant folding/propagation |
| 6 | `src/codegen.c` | Translates AST to equivalent C source code |

## Repository Structure

```
├── include/            # Header files
│   ├── common.h        #   Shared types, structs, externs
│   ├── ast.h           #   AST node constructors
│   ├── symtab.h        #   Symbol table interface
│   ├── semantic.h      #   Semantic analysis
│   ├── interpreter.h   #   Tree-walk interpreter
│   ├── tac.h           #   TAC generation
│   ├── optimizer.h     #   Code optimization
│   └── codegen.h       #   C code generation
├── src/                # Source implementations
│   ├── main.c          #   Entry point (orchestrates 6 phases)
│   ├── ast.c           #   AST node pool & constructors
│   ├── symtab.c        #   Scoped symbol table
│   ├── semantic.c      #   Static semantic checks
│   ├── interpreter.c   #   Runtime execution
│   ├── tac.c           #   Intermediate code generation
│   ├── optimizer.c     #   Constant folding, propagation, dead code
│   └── codegen.c       #   AST → C translation
├── examples/           # Example S++ programs
│   ├── 01_variables.txt ... 08_full_demo.txt  (feature demos)
│   └── err_01_undeclared.txt ... err_05_syntax.txt  (error tests)
├── spp_lexer.l         # Flex lexer specification
├── spp_parser.y        # Bison grammar specification
├── Makefile            # Build script
└── input.txt           # Default test input
```

## Build and Run

### Prerequisites
- `gcc` (MinGW on Windows)
- `flex`
- `bison`

### Build
```bash
make
```

### Run
```bash
./spp_compiler input.txt output.txt
# Or use an example:
./spp_compiler examples/01_variables.txt output.txt
```

### Clean
```bash
make clean
```

## Example S++ Program

```
purno add(purno a, purno b) shuru
    ferot a jog b;
shesh

purno main() shuru
    purno x = 10;
    purno y = 20;
    purno sum = add(x, y);
    dekhao("Sum =");
    dekhao(sum);
    ferot 0;
shesh
```
