# Pipes & Processes – Forksort

A **parallel Merge Sort** implementation that uses process forking and pipes instead of threads for concurrency.

---

## Concept

Instead of recursively sorting in a single process, `forksort` spawns child processes for each half of the data. Data flows through pipes, creating a process tree that mirrors the merge sort recursion tree.

```
                    ┌───────────┐
        Input ──────│  forksort │────── Sorted Output
                    └─────┬─────┘
                         ╱ ╲
               ┌────────┘   └────────┐
               │                     │
          ┌────┴────┐           ┌────┴────┐
          │ Child L │           │ Child R │
          └────┬────┘           └────┬────┘
              ╱ ╲                   ╱ ╲
            ...  ...              ...  ...
```

## How It Works

1. **Read Phase**: Read all input lines into memory
2. **Base Case**: If ≤1 line, output immediately
3. **Fork Phase**:
   - Create 4 pipes (2 per child: input/output)
   - Fork two children
   - Redirect child's stdin/stdout to parent's pipes
   - Child calls `execlp()` on itself
4. **Feed Phase**: Parent writes first half to left child, second half to right
5. **Merge Phase**: Parent reads sorted output from both children and merges
6. **Cleanup**: Wait for children, free memory

---



## Usage

```bash
# Sort lines from file
./forksort < input.txt

# Sort piped input
echo -e "banana\napple\ncherry" | ./forksort

# Sort and save to file
./forksort < unsorted.txt > sorted.txt
```

---

## Key Concepts

| Concept | Implementation |
|---------|----------------|
| **Process Creation** | `fork()` to spawn child processes |
| **Pipe Creation** | `pipe()` for unidirectional data channels |
| **I/O Redirection** | `dup2()` to redirect stdin/stdout |
| **Self Execution** | `execlp(argv[0], ...)` for recursive calls |
| **Zombie Prevention** | `waitpid()` to reap child processes |



---

## Build

```bash
make
```


