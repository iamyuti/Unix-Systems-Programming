/**
 * @file generator.c
 * @brief The Producer process (Worker).
 * * Responsibilities:
 * 1. Attaches to existing Shared Memory.
 * 2. Generates random 3-colorings for a given graph.
 * 3. Identifies conflicting edges (nodes with same color).
 * 4. Pushes solutions to the circular buffer if valid.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "common.h"

#define MAX_EDGES 200
#define MAX_NODES 100 // Assumption for array sizing

edge_t graph_edges[MAX_EDGES];
int num_edges = 0;
int max_node_id = 0;

// Parse "u-v" arguments from command line
void parse_graph(int argc, char *argv[]) {
    num_edges = argc - 1;
    if (num_edges > MAX_EDGES) {
        fprintf(stderr, "Error: Too many edges (limit %d)\n", MAX_EDGES);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (sscanf(argv[i], "%d-%d", &graph_edges[i-1].u, &graph_edges[i-1].v) != 2) {
            fprintf(stderr, "Invalid format: %s. Use u-v (e.g., 1-2)\n", argv[i]);
            exit(EXIT_FAILURE);
        }
        // Track highest node ID for array sizing
        if (graph_edges[i-1].u > max_node_id) max_node_id = graph_edges[i-1].u;
        if (graph_edges[i-1].v > max_node_id) max_node_id = graph_edges[i-1].v;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s edge1 edge2 ... (e.g. 0-1 1-2 2-0)\n", argv[0]);
        return EXIT_FAILURE;
    }

    parse_graph(argc, argv);
    srand(time(NULL) ^ getpid()); // Seed random number generator

    // Open Shared Memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shm_fd == -1) {
        perror("shm_open failed (Supervisor must be running)");
        return EXIT_FAILURE;
    }

    shm_t *shm_ptr = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap failed");
        return EXIT_FAILURE;
    }

    // Open Semaphores
    sem_t *sem_free = sem_open(SEM_FREE, 0);
    sem_t *sem_used = sem_open(SEM_USED, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);

    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("sem_open failed");
        return EXIT_FAILURE;
    }

    int colors[MAX_NODES];

    // Main Producer Loop
    while (!shm_ptr->terminate) {
        
        // Algorithm: Random Coloring Heuristic
        for (int i = 0; i <= max_node_id; i++) {
            colors[i] = rand() % 3; // Assign 0, 1, or 2
        }

        solution_t sol;
        sol.edge_count = 0;
        int solution_valid = 1;

        // Verify edges
        for (int i = 0; i < num_edges; i++) {
            int u = graph_edges[i].u;
            int v = graph_edges[i].v;

            if (colors[u] == colors[v]) {
                // Conflict found
                if (sol.edge_count < MAX_REMOVED_EDGES) {
                    sol.edges[sol.edge_count++] = graph_edges[i];
                } else {
                    solution_valid = 0; // Too many removed edges, discard solution
                    break;
                }
            }
        }

        if (!solution_valid) continue; // Try again with new colors
        
        // Wait for free slot
        if (sem_wait(sem_free) == -1) break; 
        if (shm_ptr->terminate) break;

        // Enter Critical Section
        sem_wait(sem_mutex);
        
        int idx = shm_ptr->write_index;
        shm_ptr->buffer[idx] = sol;
        shm_ptr->write_index = (idx + 1) % BUFFER_SIZE;
        
        sem_post(sem_mutex);
        // Leave Critical Section

        // Signal new data available
        sem_post(sem_used);
    }

    // Cleanup (detach only, do not destroy)
    munmap(shm_ptr, sizeof(shm_t));
    sem_close(sem_free);
    sem_close(sem_used);
    sem_close(sem_mutex);
    close(shm_fd);

    return EXIT_SUCCESS;
}
