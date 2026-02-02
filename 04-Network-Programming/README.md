# Network Programming

A basic **HTTP client and server** implementation demonstrating TCP socket programming with the Berkeley Sockets API.

---

## Components

### HTTP Client

A command-line tool that sends HTTP GET requests and displays the response.

### HTTP Server

A single-threaded web server that serves static files from the current directory.

---

## Usage

### HTTP Client

```bash
./http_client <hostname> <path> [port]
```

#### Examples

```bash
# Fetch example.com homepage
./http_client www.example.com /

# Fetch specific resource on custom port
./http_client localhost /index.html 8080
```

### HTTP Server

```bash
./http_server [port]
```

Default port: `8080`

#### Examples

```bash
# Start server on default port
./http_server

# Start server on port 3000
./http_server 3000

# Then access via browser: http://localhost:8080
```

---



## Architecture

### Client Flow

```
1. Resolve hostname     → getaddrinfo()
2. Create socket        → socket()
3. Connect to server    → connect()
4. Wrap as stream       → fdopen()
5. Send HTTP GET        → fprintf()
6. Read response        → fgets()
```

### Server Flow

```
1. Create socket        → socket()
2. Set reuse option     → setsockopt(SO_REUSEADDR)
3. Bind to port         → bind()
4. Start listening      → listen()
5. Accept connection    → accept()
6. Handle request       → parse + serve_file()
7. Close connection     → fclose()
8. Loop to step 5
```

---

## Server Features

| Feature | Description |
|---------|-------------|
| **Static file serving** | Serves files from current directory |
| **Path traversal protection** | Blocks `..` in paths |
| **Default document** | Serves `index.html` for `/` |
| **Error responses** | 400, 403, 404, 501 status codes |
| **Port reuse** | `SO_REUSEADDR` for quick restarts |

---

## Build

```bash
make
```

---


