/**
 * @file supervisor.c
 * @brief The Consumer process (Host).
 * * Responsibilities:
 * 1. Sets up POSIX Shared Memory and Semaphores.
 * 2. Parses command line arguments (-n limit, -w delay).
 * 3. Reads proposed solutions from the circular buffer.
 * 4. Tracks the best solution and shuts down gracefully.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <errno.h> // Required for errno
#include "common.h"

// Global pointers for signal handler cleanup
shm_t *shm_ptr = NULL;
sem_t *sem_free = NULL;
sem_t *sem_used = NULL;
sem_t *sem_mutex = NULL;
int shm_fd = -1;

/**
 * @brief Clean up resources (Unlink SHM/Sems, Close FDs)
 */
void cleanup(void) {
    // Signal generators to stop
    if (shm_ptr != NULL) {
        shm_ptr->terminate = 1;
    }

    // Wake up potentially sleeping generators so they can terminate
    if (sem_free) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            sem_post(sem_free); 
        }
        sem_close(sem_free);
        sem_unlink(SEM_FREE);
    }
    
    if (sem_used) {
        sem_close(sem_used);
        sem_unlink(SEM_USED);
    }
    
    if (sem_mutex) {
        sem_close(sem_mutex);
        sem_unlink(SEM_MUTEX);
    }

    if (shm_ptr != NULL) {
        munmap(shm_ptr, sizeof(shm_t));
    }
    
    if (shm_fd != -1) {
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }
}

void handle_signal(int signal) {
    (void)signal; // suppress unused warning
    cleanup();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    // Unbuffered output for real-time logging
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    // Register signal handlers
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Argument Parsing
    int limit = INT_MAX;
    int delay = 0;
    int opt;
    char *endptr;

    while ((opt = getopt(argc, argv, "n:w:")) != -1) {
        switch (opt) {
            case 'n': // limit solutions
                errno = 0;
                long val_n = strtol(optarg, &endptr, 10);
                if (errno != 0 || *endptr != '\0' || val_n <= 0) {
                    fprintf(stderr, "Error: Invalid number for limit (-n)\n");
                    return EXIT_FAILURE;
                }
                limit = (int)val_n;
                break;
            case 'w': // wait delay
                errno = 0;
                long val_w = strtol(optarg, &endptr, 10);
                if (errno != 0 || *endptr != '\0' || val_w < 0) {
                    fprintf(stderr, "Error: Invalid number for delay (-w)\n");
                    return EXIT_FAILURE;
                }
                delay = (int)val_w;
                break;
            default:
                fprintf(stderr, "Usage: %s [-n limit] [-w delay]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Create Shared Memory
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return EXIT_FAILURE;
    }

    if (ftruncate(shm_fd, sizeof(shm_t)) == -1) {
        perror("ftruncate failed");
        cleanup(); // Cleanup early if resize fails
        return EXIT_FAILURE;
    }

    shm_ptr = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap failed");
        cleanup();
        return EXIT_FAILURE;
    }

    // Initialize Shared Memory State
    shm_ptr->terminate = 0;
    shm_ptr->write_index = 0;
    shm_ptr->read_index = 0;

    // Create Semaphores
    sem_free = sem_open(SEM_FREE, O_CREAT, 0600, BUFFER_SIZE);
    sem_used = sem_open(SEM_USED, O_CREAT, 0600, 0);
    sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0600, 1);

    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("sem_open failed");
        cleanup();
        return EXIT_FAILURE;
    }

    printf("[Supervisor] System initialized. Waiting for solutions...\n");

    // Optional start delay
    if (delay > 0) {
        printf("[Supervisor] Sleeping for %d seconds...\n", delay);
        sleep(delay);
    }

    // Processing Loop
    int solutions_read = 0;
    int best_edge_count = MAX_REMOVED_EDGES + 1;
    
    // Loop continues until limit reached OR termination signal
    while (!shm_ptr->terminate && solutions_read < limit) {
        
        // Wait for a solution (Consumer wait)
        if (sem_wait(sem_used) == -1) {
            if (errno == EINTR) break; // Interrupted by signal
            continue;
        }

        // Read from buffer
        int idx = shm_ptr->read_index;
        solution_t sol = shm_ptr->buffer[idx];
        shm_ptr->read_index = (idx + 1) % BUFFER_SIZE;

        // Signal free slot
        sem_post(sem_free);
        solutions_read++;

        // Check if solution is better
        if (sol.edge_count < best_edge_count) {
            best_edge_count = sol.edge_count;
            printf("[Supervisor] New best solution found! Removed edges: %d\n", best_edge_count);

            if (best_edge_count == 0) {
                printf("[Supervisor] Graph is 3-colorable! Terminating.\n");
                break;
            }
        }
    }

    if (best_edge_count > 0 && best_edge_count <= MAX_REMOVED_EDGES) {
        printf("[Supervisor] Finished. Best solution removes %d edges.\n", best_edge_count);
    }

    cleanup();
    return EXIT_SUCCESS;
}
