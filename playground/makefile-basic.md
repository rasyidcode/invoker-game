# Makefile Basic

## The core Anatomy: A Rule

Every Makefile is built out of *rules*. A rule always follows this exact structure:

```Makefile
target: prerequisites
    command
```

- Target: What to build (usually an executable or an object file like app or main.o), or an action name (like clean).
- Prerequisites (Dependencies): The files that the target depends on. If any prerequisite is newer than target, make runs the command.
- Command (Recipe): The shell command to run.

*The Golden Rule of Makefiles": The indentation before commands MUST be a real Tab character (\t), never spaces! If use spaces, make will fail with an error like missing separator.
