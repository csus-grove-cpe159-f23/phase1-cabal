/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel Semaphores
 */

#include <spede/string.h>

#include "kernel.h"
#include "ksem.h"
#include "queue.h"
#include "scheduler.h"

// Table of all semephores
sem_t semaphores[SEM_MAX];

// semaphore ids to be allocated
queue_t sem_queue;

/**
 * Initializes kernel semaphore data structures
 * @return -1 on error, 0 on success
 */
int ksemaphores_init() {
    kernel_log_info("Initializing kernel semaphores");

    // Initialize the semaphore table

    // Initialize the semaphore queue

    // Fill the semaphore queue

    return 0;
}

/**
 * Allocates a semaphore
 * @param value - initial semaphore value
 * @return -1 on error, otherwise the semaphore id that was allocated
 */
int ksem_init(int value) {
    // Obtain a semaphore id from the semaphore queue

    // Ensure that the id is within the valid range

    // Initialize the semaphore data structure
    // sempohare table + all members (wait queue, allocated, count)
        // set count to initial value

    return -1;
}

/**
 * Frees the specified semaphore
 * @param id - the semaphore id
 * @return 0 on success, -1 on error
 */
int ksem_destroy(int id) {
    // look up the sempaphore in the semaphore table

    // If the semaphore is locked, prevent it from being destroyed

    // Add the id back into the semaphore queue to be re-used later

    // Clear the memory for the data structure

    return -1;
}

/**
 * Waits on the specified semaphore if it is held
 * @param id - the semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int ksem_wait(int id) {
    // look up the sempaphore in the semaphore table

    // If the semaphore count is 0, then the process must wait
        // Set the state to WAITING
        // add to the semaphore's wait queue
        // remove from the scheduler

    // If the semaphore count is > 0
        // Decrement the count

    // Return the current semaphore count

    return -1;
}

/**
 * Posts the specified semaphore
 * @param id - the semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int ksem_post(int id) {

    // look up the semaphore in the semaphore table

    // incrememnt the semaphore count

    // check if any processes are waiting on the semaphore (semaphore wait queue)
        // if so, queue out and add to the scheduler
        // decrement the semaphore count

    // return current semaphore count

    return -1;
}
