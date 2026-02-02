# Shared Memory Synchronisation

A **producer-consumer** system for solving the *Graph 3-Coloring Problem* using POSIX shared memory and semaphores for inter-process communication.

---

## Problem Description

Given a graph, determine if it can be properly 3-colored (no two adjacent vertices share the same color). The system uses multiple **generator processes** to propose solutions, which are evaluated by a single **supervisor process**.

```
┌─────────────┐     Shared Memory      ┌────-────────-┐
│  Generator  │ ──────────────────────▶│  Supervisor  │
│  (Producer) │    [Circular Buffer]   │  (Consumer)  │
└─────────────┘                        └──────────--──┘
       │                                       │
       └───────--─ Semaphores (sync) ───────-──┘
```

---

## Features

- **POSIX Shared Memory** for zero-copy IPC
- **Named Semaphores** for safe synchronization
- **Circular Buffer** for lock-free read/write logic
- **Graceful shutdown** via signal handling (`SIGINT`, `SIGTERM`)
- **Parallel execution** with multiple generators

---

## Usage

### Start the Supervisor (must run first)

```bash
./supervisor [-n limit] [-w delay]
```

| Option | Description |
|--------|-------------|
| `-n N` | Stop after reading N solutions |
| `-w S` | Wait S seconds before starting |

### Start Generators (in separate terminals)

```bash
./generator edge1 edge2 edge3 ...
```

Edges are specified as `u-v` pairs (e.g., `0-1 1-2 2-0` for a triangle).

### Example

```bash
# Terminal 1: Start supervisor
./supervisor -n 100

# Terminal 2 & 3: Start generators for a graph
./generator 0-1 1-2 2-3 3-0 0-2
./generator 0-1 1-2 2-3 3-0 0-2
```

---

## Key Concepts

| Concept | Implementation |
|---------|----------------|
| **Shared Memory** | `shm_open()`, `mmap()`, `ftruncate()` |
| **Semaphores** | `sem_open()`, `sem_wait()`, `sem_post()` |
| **Circular Buffer** | Read/write indices with modulo arithmetic |
| **Signal Handling** | `sigaction()` for clean resource cleanup |
| **Producer-Consumer** | Classic synchronization pattern |

---

## Semaphore System

| Semaphore | Purpose | Initial Value |
|-----------|---------|---------------|
| `SEM_FREE` | Counts free buffer slots | `BUFFER_SIZE` |
| `SEM_USED` | Counts available solutions | `0` |
| `SEM_MUTEX` | Protects write access | `1` |

---

## Build

```bash
make
```
