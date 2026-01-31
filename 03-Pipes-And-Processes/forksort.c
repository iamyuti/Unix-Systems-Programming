/**
 * @file forksort.c
 * @brief A parallel Merge Sort implementation using Processes and Pipes.
 * * Instead of threads, this program uses fork() to create child processes.
 * Data is passed between processes via STDIN/STDOUT pipes.
 * Recursion is achieved by the process calling execlp() on itself.
 * * Logic:
 * 1. Read input from STDIN.
 * 2. If lines > 1:
 * - Split lines into two halves.
 * - Fork two children.
 * - Pipe first half to Left Child.
 * - Pipe second half to Right Child.
 * - Read sorted output from both children.
 * - Merge the sorted streams and print to STDOUT.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

// Helper to free memory
void free_lines(char **lines, size_t count) {
    if (lines) {
        for (size_t i = 0; i < count; i++) {
            free(lines[i]);
        }
        free(lines);
    }
}

int main(int argc, char *argv[]) {
    // Suppress unused parameter warning
    (void)argc;

    // Read all lines from STDIN dynamically
    char **lines = NULL;
    size_t count = 0;
    size_t capacity = 0;
    char buffer[4096]; // Large buffer for safety

    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (count == capacity) {
            capacity = (capacity == 0) ? 16 : capacity * 2;
            char **new_lines = realloc(lines, capacity * sizeof(char *));
            if (!new_lines) {
                perror("realloc failed");
                free_lines(lines, count);
                exit(EXIT_FAILURE);
            }
            lines = new_lines;
        }
        lines[count] = strdup(buffer);
        if (!lines[count]) {
            perror("strdup failed");
            exit(EXIT_FAILURE);
        }
        count++;
    }

    // Base Case: If 0 or 1 line, it is already sorted.
    if (count <= 1) {
        if (count == 1) {
            printf("%s", lines[0]);
        }
        free_lines(lines, count);
        return EXIT_SUCCESS;
    }

    // Prepare Pipes
    // pipe_to_left[0] = read end (child uses), pipe_to_left[1] = write end (parent uses)
    int pipe_to_left[2], pipe_from_left[2];
    int pipe_to_right[2], pipe_from_right[2];

    if (pipe(pipe_to_left) == -1 || pipe(pipe_from_left) == -1 ||
        pipe(pipe_to_right) == -1 || pipe(pipe_from_right) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    // Fork Left Child
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        // Child 1 Configuration
        // Redirect STDIN to read from parent
        dup2(pipe_to_left[0], STDIN_FILENO);
        // Redirect STDOUT to write to parent
        dup2(pipe_from_left[1], STDOUT_FILENO);

        // Close all pipe ends (duplicates are now active on 0 and 1)
        close(pipe_to_left[0]); close(pipe_to_left[1]);
        close(pipe_from_left[0]); close(pipe_from_left[1]);
        close(pipe_to_right[0]); close(pipe_to_right[1]);
        close(pipe_from_right[0]); close(pipe_from_right[1]);

        // Recursively execute self
        execlp(argv[0], argv[0], NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    }

    // Fork Right Child
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        // Child 2 Configuration
        dup2(pipe_to_right[0], STDIN_FILENO);
        dup2(pipe_from_right[1], STDOUT_FILENO);

        close(pipe_to_left[0]); close(pipe_to_left[1]);
        close(pipe_from_left[0]); close(pipe_from_left[1]);
        close(pipe_to_right[0]); close(pipe_to_right[1]);
        close(pipe_from_right[0]); close(pipe_from_right[1]);

        execlp(argv[0], argv[0], NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    }

    // Close unused pipe ends immediately
    close(pipe_to_left[0]);   // Parent only writes to children
    close(pipe_from_left[1]); // Parent only reads from children
    close(pipe_to_right[0]);
    close(pipe_from_right[1]);

    // Feed data to children
    // Left half
    for (size_t i = 0; i < count / 2; i++) {
        dprintf(pipe_to_left[1], "%s", lines[i]);
    }
    close(pipe_to_left[1]); // EOF for Left Child

    // Right half
    for (size_t i = count / 2; i < count; i++) {
        dprintf(pipe_to_right[1], "%s", lines[i]);
    }
    close(pipe_to_right[1]); // EOF for Right Child

    // Read sorted output from children
    FILE *f_left = fdopen(pipe_from_left[0], "r");
    FILE *f_right = fdopen(pipe_from_right[0], "r");

    char buf_left[4096];
    char buf_right[4096];

    char *res_left = fgets(buf_left, sizeof(buf_left), f_left);
    char *res_right = fgets(buf_right, sizeof(buf_right), f_right);

    while (res_left && res_right) {
        if (strcmp(buf_left, buf_right) <= 0) {
            printf("%s", buf_left);
            res_left = fgets(buf_left, sizeof(buf_left), f_left);
        } else {
            printf("%s", buf_right);
            res_right = fgets(buf_right, sizeof(buf_right), f_right);
        }
    }

    // Flush remaining lines
    while (res_left) {
        printf("%s", buf_left);
        res_left = fgets(buf_left, sizeof(buf_left), f_left);
    }
    while (res_right) {
        printf("%s", buf_right);
        res_right = fgets(buf_right, sizeof(buf_right), f_right);
    }

    // Cleanup
    fclose(f_left);
    fclose(f_right);
    free_lines(lines, count);

    // Wait for children to avoid zombies
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return EXIT_SUCCESS;
}
