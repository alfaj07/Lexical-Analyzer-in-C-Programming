# Lexical Analyzer in C

[![Language](https://img.shields.io/badge/Language-C-00599C.svg?logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Compiler](https://img.shields.io/badge/Compiler-GCC%20%7C%20Clang-brightgreen.svg?logo=gnu&logoColor=white)](https://gcc.gnu.org/)
[![Build](https://img.shields.io/badge/Build-GNU%20Make-4EAA25.svg?logo=gnu&logoColor=white)](https://www.gnu.org/software/make/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20WSL%20%7C%20Windows-orange.svg)](https://github.com/alfaj07/Lexical-Analyzer-in-C-Programming)
[![License](https://img.shields.io/badge/License-MIT-purple.svg)](LICENSE)

A lightweight, hand-crafted Lexical Analyzer (lexer/scanner) implemented in standard C. This tool reads C source files and converts the character stream into categorized tokens such as keywords, identifiers, constants/literals, operators, and special characters. It also performs syntax and lexical validation, reporting errors and warnings with exact line numbers.

---

## Overview

Lexical analysis represents the initial front-end phase of a compiler. The lexer processes the input source program, filters out whitespace, identifies individual lexemes, and maps them to their respective token classifications.

### Supported Token Categories
- **Keywords**: Reserved C language constructs (`int`, `float`, `return`, `if`, `else`, `while`, `for`, `do`, `break`, `continue`, `char`, `double`, `void`, `switch`, `case`, `default`, `const`, `static`, `sizeof`, `struct`).
- **Identifiers**: Names representing variables, functions, and user-defined symbols (`[a-zA-Z_][a-zA-Z0-9_]*`).
- **Constants / Literals**: Numeric constants (integers, floating-point numbers), character literals (`'c'`), and string literals (`"Hello World"`).
- **Operators**: Arithmetic, relational, logical, and assignment symbols (`+`, `-`, `*`, `/`, `%`, `=`, `!`, `<`, `>`, `|`, `&`, `==`, `!=`, `<=`, `>=`, `&&`, `||`, `++`, `--`, etc.).
- **Special Characters**:
  - Delimiters, punctuation, and scope braces (`,`, `;`, `{`, `}`, `(`, `)`, `[`, `]`).
  - Escape sequences (`\n`, `\t`, `\r`, `\0`, `\a`, `\b`, `\f`, `\v`, `\\`).

---

## Key Features

1. **Escape Sequence Extraction**: Escape sequences inside string literals (e.g., `\n`, `\t`, `\r`) are automatically separated and classified as individual `SPECIAL_CHARACTER` tokens, while the surrounding text is preserved as a `CONSTANT`.
2. **Preprocessor Directive Handling**: Safely ignores preprocessor lines starting with `#` (e.g., `#include <stdio.h>`, `#define MAX 100`) like comments, preventing premature termination.
3. **Comment Stripping**: Automatically skips both single-line (`// ...`) and multi-line (`/* ... */`) comments.
4. **Line Number Tracking**: Accurately tracks source code line numbers across whitespace, directives, comments, and strings.
5. **Warning & Error Summary Table**: Automatically detects syntax and lexical issues (e.g., missing closing parenthesis `)` before `;`, unclosed strings, or unknown symbols) and displays a secondary summary table with exact line numbers.

---

## Project Structure

```text
.
├── Makefile          # Build automation configuration (with test exclusion)
├── lexer.h           # Token types, definitions, structures, and prototypes
├── lexer.c           # Scanning routines, escape extraction, line tracking, and classification
├── main.c            # CLI entry point, table formatting, and warning summary report
├── samples/          # Test input programs (e.g., test.c)
├── LICENSE           # Open-source license (MIT)
└── README.md         # Architecture and usage documentation
```

---

## Token Classifications

| Token Type | Enum Code | Description | Sample Lexemes |
| :--- | :---: | :--- | :--- |
| `KEYWORD` | `0` | Reserved language constructs | `int`, `return`, `while`, `struct` |
| `OPERATOR` | `1` | Arithmetic, relational, and logical symbols | `+`, `-`, `*`, `/`, `=`, `==`, `<` |
| `SPECIAL_CHARACTER` | `2` | Brackets, delimiters, punctuation & escape sequences | `{`, `}`, `(`, `)`, `;`, `,`, `\n`, `\t` |
| `CONSTANT` | `3` | Numeric, character, or string literals | `0`, `3.14`, `'c'`, `"Hello World"` |
| `IDENTIFIER` | `4` | Variable and function identifiers | `main`, `printf`, `count`, `sum` |
| `UNKNOWN` | `5` | Unrecognized tokens or end-of-stream indicator | `@`, `$`, `~` |

---

## File Overview

- **`Makefile`**: Automates compilation, object management, and cleaning targets. Automatically excludes standalone test files (`test.c`) from being linked into the analyzer.
- **`lexer.h`**: Declares the `TokenType` enumeration, the `Token` data structure (`lexeme`, `type`, `warning`, `line`), token buffer limits, and function prototypes.
- **`lexer.c`**: Implements scanning logic, escape sequence separation, preprocessor line handling, line tracking, keyword lookup, delimiter matching, and token categorization.
- **`main.c`**: Parses command-line arguments, triggers `initializeLexer()`, queries tokens via `getNextToken()`, outputs the formatted primary token table, and conditionally displays the secondary warning/error summary table.
- **`samples/`**: Stores input source files (e.g., `test.c`) used for testing and validation.

---

## Build & Run Instructions

### Prerequisites
- GCC or any standard C compatible compiler
- GNU Make

### Compilation

Compile the project using `make`:

```bash
make
```

To clean generated binaries and object files:

```bash
make clean
```

### Execution

Execute the binary by passing a C source file path as an argument:

```bash
./lexical_analyzer samples/test.c
```

---

## Example Runs

### 1. Standard Run (Valid C Code)

#### Input File (`samples/test.c`)

```c
#include <stdio.h>

int main() {
    printf("Hello World\n");
    return 0;
}
```

#### Program Output

```text
+----------------------+--------------------+-------+
| Lexeme               | Token Type Name    | Code  |
+----------------------+--------------------+-------+
| int                  | KEYWORD            |   0   |
| main                 | IDENTIFIER         |   4   |
| (                    | SPECIAL_CHARACTER  |   2   |
| )                    | SPECIAL_CHARACTER  |   2   |
| {                    | SPECIAL_CHARACTER  |   2   |
| printf               | IDENTIFIER         |   4   |
| (                    | SPECIAL_CHARACTER  |   2   |
| "Hello World"        | CONSTANT           |   3   |
| \n                   | SPECIAL_CHARACTER  |   2   |
| )                    | SPECIAL_CHARACTER  |   2   |
| ;                    | SPECIAL_CHARACTER  |   2   |
| return               | KEYWORD            |   0   |
| 0                    | CONSTANT           |   3   |
| ;                    | SPECIAL_CHARACTER  |   2   |
| }                    | SPECIAL_CHARACTER  |   2   |
+----------------------+--------------------+-------+
```

---

### 2. Warning / Error Detection Run

When syntax irregularities or unclosed delimiters are encountered:

#### Input File (Missing Closing Parenthesis `)`)

```c
#include <stdio.h>

int main() {
    printf("Hello World\n";
    return 0;
}
```

#### Program Output

```text
+----------------------+--------------------+-------+
| Lexeme               | Token Type Name    | Code  |
+----------------------+--------------------+-------+
| int                  | KEYWORD            |   0   |
| main                 | IDENTIFIER         |   4   |
| (                    | SPECIAL_CHARACTER  |   2   |
| )                    | SPECIAL_CHARACTER  |   2   |
| {                    | SPECIAL_CHARACTER  |   2   |
| printf               | IDENTIFIER         |   4   |
| (                    | SPECIAL_CHARACTER  |   2   |
| "Hello World"        | CONSTANT           |   3   |
| \n                   | SPECIAL_CHARACTER  |   2   |
| ;                    | SPECIAL_CHARACTER  |   2   |
| return               | KEYWORD            |   0   |
| 0                    | CONSTANT           |   3   |
| ;                    | SPECIAL_CHARACTER  |   2   |
| }                    | SPECIAL_CHARACTER  |   2   |
+----------------------+--------------------+-------+

+---------+----------------------+------------------------------------+
| Line No | Lexeme               | Warning / Error Description        |
+---------+----------------------+------------------------------------+
| 4       | ;                    | Missing ')' before ';'             |
+---------+----------------------+------------------------------------+
```

---

## License

This project is licensed under the [MIT License](LICENSE).
