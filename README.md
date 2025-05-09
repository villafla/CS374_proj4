# 🐚 smallsh – A Custom Linux Shell in C

## 📘 Overview

`smallsh` is a custom Linux shell written in C that supports a subset of features found in modern shells like Bash. It provides a prompt to execute commands, handle foreground/background processes, implement redirection, and manage signals—offering a hands-on experience with process control, I/O management, and signal handling using the Linux API.

This project was designed as the final portfolio assignment for CS 374 and demonstrates key systems programming concepts in action.

## 🧠 Features

- `:` prompt for interactive shell input
- Built-in support for the commands:  
  - `exit`: Terminates the shell and kills background processes  
  - `cd`: Changes the working directory  
  - `status`: Reports exit status or signal termination info
- Executes non-built-in commands via `fork()` + `execvp()`
- Input/output redirection via `<`, `>` using `dup2()`
- Background execution using `&` and process management with `waitpid()`
- Foreground-only mode toggle using `SIGTSTP` (Ctrl+Z)
- Proper handling of `SIGINT` (Ctrl+C) for foreground-only processes
- Shell ignores blank lines and comment lines beginning with `#`

## ⚙️ Compilation

To compile the program, run:

```bash
gcc --std=gnu99 -o smallsh *.c
```

## 🚀 How to Run

```bash
./smallsh
```

## 📌 Example Usage

```bash
: ls
: pwd
: cd ..
: echo Hello > file.txt
: cat < file.txt
: sleep 10 &
: status
: exit
```
## 📎 Technical Highlights
- Processes: Uses fork(), execvp(), and waitpid() to manage execution
- Redirection: Handles input/output with dup2() and file descriptors
- Signals:
  - Ignores SIGINT in the parent shell but allows it to terminate foreground children
  - Handles SIGTSTP to toggle foreground-only mode with a custom message
- Background jobs: PIDs tracked and reaped with waitpid(...WNOHANG...)

## 🧪 Test Cases & Example Run
Test the following scenarios:
- ✅ Built-in commands: cd, status, exit
- ✅ I/O redirection: cat < input.txt > output.txt
- ✅ Background jobs: sleep 5 &
- ✅ Signal handling: Ctrl+C, Ctrl+Z
