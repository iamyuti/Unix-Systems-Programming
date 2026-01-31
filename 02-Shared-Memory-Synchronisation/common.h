/**
 * @file common.h
 * @brief Shared definitions for the Graph Coloring IPC system.
 * Defines the circular buffer structure and semaphore constants used for synchronization.
 */

#ifndef COMMON_H 
#define COMMON_H

#include <semaphore.h>

// Shared Memory and Semaphore names (POSIX standard)
#define SHM_NAME "/graph_coloring_shm"
#define SEM_FREE "/graph_coloring_sem_free"   // Counts free slots in buffer
#define SEM_USED "/graph_coloring_sem_used"   // Counts used slots (solutions) in buffer
#define SEM_MUTEX "/graph_coloring_sem_mutex" // Protects write access

#define BUFFER_SIZE 20
#define MAX_REMOVED_EDGES 8

typedef struct {
    int u;
    int v;
} edge_t;

/**
 * @brief Represents a proposed solution to the coloring problem.
 * Contains only the edges that had to be removed to make the graph 3-colorable.
 */
typedef struct {
    int edge_count;
    edge_t edges[MAX_REMOVED_EDGES]; 
} solution_t;

/**
 * @brief The Shared Memory layout.
 * Implements a circular buffer for lock-free reading/writing logic combined with semaphores.
 */
typedef struct {
    volatile int terminate;  // Atomic flag to signal shutdown to all processes
    int write_index;         // Producer's head
    int read_index;          // Consumer's tail
    solution_t buffer[BUFFER_SIZE]; 
} shm_t;

#endif
