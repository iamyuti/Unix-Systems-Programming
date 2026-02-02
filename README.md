# Unix Systems Programming

A collection of low-level systems programming projects implemented in **C**, demonstrating core Unix/POSIX concepts essential for embedded systems, operating systems, and high-performance computing.

---

## Project Overview

| Project | Key Topics |
|---------|------------|
| [**01-CLI-Tools**](./01-CLI-Tools) | POSIX argument parsing (`getopt`), stream processing, dynamic memory |
| [**02-Shared-Memory-Synchronisation**](./02-Shared-Memory-Synchronisation) | POSIX Shared Memory, named semaphores, producer-consumer pattern |
| [**03-Pipes-And-Processes**](./03-Pipes-And-Processes) | `fork()`, `pipe()`, `execlp()`, recursive process spawning |
| [**04-Network-Programming**](./04-Network-Programming) | TCP sockets, DNS resolution, HTTP protocol implementation |

---

## Building & Running

Each project includes a `Makefile`. Navigate to the project directory and run:

```bash
# Compile
make

# Run (see individual READMEs for usage)
./program_name [args]

# Clean build artifacts
make clean
```

---

## Requirements

- **GCC** (or any C99-compliant compiler)
- **Unix/Linux/macOS** environment
- POSIX-compatible system libraries
