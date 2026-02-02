# CLI Tools – mygrep

A simplified implementation of the Unix `grep` utility, demonstrating POSIX-compliant command-line argument parsing and stream processing.

---

## Features

- **Case-insensitive search** (`-i` flag)
- **Custom output file** (`-o` option)
- **Multiple input files** support
- **Standard input** processing when no files are specified
- **Graceful error handling** with continuation on file errors

---

## Usage

```bash
./mygrep [-i] [-o outfile] keyword [file...]
```

### Options

| Option | Description |
|--------|-------------|
| `-i` | Perform case-insensitive matching |
| `-o FILE` | Write output to FILE instead of stdout |

### Examples

```bash
# Search for "error" in log.txt
./mygrep error log.txt

# Case-insensitive search across multiple files
./mygrep -i warning file1.txt file2.txt

# Pipe input and save output
cat server.log | ./mygrep -i -o errors.txt exception
```

---



## Build

```bash
make
```

---


