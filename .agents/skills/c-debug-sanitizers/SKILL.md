---
name: c-debug-sanitizers
description: >-
  Runbooks and diagnostics for debugging C code, segfaults, memory leaks,
  buffer overflows, compiler warnings, and runtime bugs using ASan, UBSan,
  Valgrind, and GDB. Use when compiler errors, crashes, or memory issues occur.
---

# C Debugging & Sanitizers Skill

This skill provides step-by-step diagnostic workflows for identifying, explaining, and isolating memory errors, crashes, and compiler diagnostics in C game development.

---

## 1. Quick Debug Build Helper

To build the project with AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), and full debug symbols (`-g`):

```bash
./.agents/skills/c-debug-sanitizers/scripts/build_debug.sh
```

Then run the debug binary:
```bash
./invoker_game_debug
```

When an illegal memory access or undefined behavior occurs, ASan will halt the program immediately and print a detailed stack trace pinpointing the exact file and line number.

---

## 2. Reading AddressSanitizer (ASan) Reports

### 2.1 Stack / Global / Heap Buffer Overflow
- **Indicator**: `ERROR: AddressSanitizer: stack-buffer-overflow` or `global-buffer-overflow`
- **Cause**: Reading or writing past the boundary of an array.
  - *Example in Invoker*: Accessing `buffer->orbs[3]` in a 3-element array (`orbs[0]`, `orbs[1]`, `orbs[2]`).
- **How to Explain to User**: Point out the array size and the index evaluated at runtime. Emphasize that in C, 0-indexed arrays with size $N$ only permit indices $0 \dots N-1$.

### 2.2 Null Pointer Dereference / Segmentation Fault
- **Indicator**: `AddressSanitizer: SEGV on unknown address 0x000000000000`
- **Cause**: Attempting to read `ptr->member` when `ptr == NULL`.
- **How to Explain to User**: Trace where `ptr` was initialized or passed. Guide the user to add defensive null checks (`if (ptr == NULL) return;`) or verify pointer initialization.

### 2.3 Use-After-Free & Double-Free
- **Indicator**: `heap-use-after-free` or `attempting double-free`
- **Cause**: Using a pointer after calling `free()` or calling `free()` twice on the same address.
- **How to Explain to User**: Explain pointer lifetime and suggest setting pointers to `NULL` immediately after freeing (`free(p); p = NULL;`).

---

## 3. Compiler Warning Diagnostics

Always treat compiler warnings as errors (`-Wall -Wextra`). Common C warnings and their remedies:

| Warning | Root Cause | Pedagogical Explanation |
| :--- | :--- | :--- |
| `implicit declaration of function 'XYZ'` | Missing header `#include` or prototype | Explain that C assumes undeclared functions return `int`. Ensure the corresponding `.h` is included. |
| `uninitialized variable 'var'` | Variable used before assignment | Stack variables hold garbage. Always initialize: `int count = 0;` or `Struct s = {0};`. |
| `comparison between signed and unsigned` | Comparing `int` with `size_t` or `unsigned int` | Negative signed integers can produce unexpected results when promoted to unsigned. Guide user on type matching. |
| `format '%d' expects argument of type 'int'` | Mismatched format specifier in `printf`/`TextFormat` | Review `printf` format strings: `%d` (int), `%f` (float/double), `%s` (char*), `%zu` (size_t). |
| `unused parameter 'param'` | Function doesn't use an argument | Explain `(void)param;` idiom to silence intentionally unused parameters (e.g., in Raylib callbacks). |

---

## 4. GDB Crash Investigation Runbook

When a program crashes without ASan:

1. **Launch under GDB**:
   ```bash
   gdb ./invoker_game
   ```
2. **Run until crash**:
   ```text
   (gdb) run
   ```
3. **Print the backtrace**:
   ```text
   (gdb) bt
   ```
   *Look for the innermost frame that belongs to `src/*.c`.*
4. **Inspect variables**:
   ```text
   (gdb) frame <N>
   (gdb) print my_variable
   (gdb) print *pointer_to_struct
   ```
5. **Exit GDB**:
   ```text
   (gdb) quit
   ```

---

## 5. Valgrind Memory Leak Checking

To check for unfreed heap allocations:
```bash
valgrind --leak-check=full --show-leak-kinds=all ./invoker_game
```
*Note: Raylib and OpenGL drivers may report small one-time allocations. Focus on leaks inside functions defined in `src/`.*
