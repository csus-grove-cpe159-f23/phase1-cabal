/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel Mutexes
 */

#include <spede/string.h>

#include "kernel.h"
#include "kmutex.h"
#include "queue.h"
#include "scheduler.h"

// Table of all mutexes
mutex_t mutexes[MUTEX_MAX];

// Mutex ids to be allocated
queue_t mutex_queue;

/**
 * Initializes kernel mutex data structures
 * @return -1 on error, 0 on success
 */
int kmutexes_init() {
    kernel_log_info("Initializing kernel mutexes");

    // Initialize the mutex table
    for (int i = 0; i < MUTEX_MAX; ++i) {
        mutexes[i].allocated = 0;  // Not allocated
        mutexes[i].locks = 0;      // No locks held
        mutexes[i].owner = NULL;   // No owner
        queue_init(&mutexes[i].wait_queue); // Initialize wait queue
    }

    // Initialize the mutexkmutex_init queue
    queue_init(&mutex_queue);

    // Fill the mutex queue with available mutex IDs
    for (int i = 0; i < MUTEX_MAX; ++i) {
        queue_in(&mutex_queue, i);
    }

    return 0;
}

/**
 * Allocates a mutex
 * @return -1 on error, otherwise the mutex id that was allocated
 */
int kmutex_init(void) {
    int id;

    // Obtain a mutex id from the mutex queue
    if (queue_out(&mutex_queue, &id) == -1) return -1; // No available mutex IDs

    // Ensure that the id is within the valid range
    if (id < 0 || id >= MUTEX_MAX) return -1;

    // Pointer to the mutex table entry
    mutex_t *mutex = &mutexes[id];

    // Initialize the mutex data structure
    mutex->allocated = 1;   // Allocated
    mutex->locks = 0;       // No locks held
    mutex->owner = NULL;    // No owner
    queue_init(&mutex->wait_queue); // Initialize wait queue

    // Return the mutex id
    return id;
}


/**
 * Frees the specified mutex
 * @param id - the mutex id
 * @return 0 on success, -1 on error
 */
int kmutex_destroy(int id) {
    // Look up the mutex in the mutex table
    if (id < 0 || id >= MUTEX_MAX || !mutexes[id].allocated) return -1;

    // If the mutex is locked, prevent it from being destroyed (return error)
    if (mutexes[id].locks > 0) return -1;

    // Add the id back into the mutex queue to be re-used later
    queue_in(&mutex_queue, id);

    // Clear the memory for the data structure
    mutexes[id].allocated = 0;

    return 0;
}


/**
 * Locks the specified mutex
 * @param id - the mutex id
 * @return -1 on error, otherwise the current lock count
 */
int kmutex_lock(int id) {
    // look up the mutex in the mutex table
    if (id < 0 || id >= MUTEX_MAX || !mutexes[id].allocated) return -1;
    // If the mutex is already locked
    //   1. Set the active process state to WAITING
    //   2. Add the process to the mutex wait queue (so it can take
    //      the mutex when it is unlocked)
    //   3. Remove the process from the scheduler, allow another
    //      process to be scheduled
    if(mutexes[id].locks > 0){
        queue_in(&mutex_queue, id);
        active_proc->state = WAITING;
        scheduler_remove(active_proc);
    }else{
        mutexes[id].owner = active_proc;
        ++mutexes[id].locks;
    }
    // If the mutex is not locked
    //   1. set the mutex owner to the active process

    // Increment the lock count

    // Return the mutex lock count

    return mutexes[id].locks;
}

/**
 * Unlocks the specified mutex
 * @param id - the mutex id
 * @return -1 on error, otherwise the current lock count
 */
int kmutex_unlock(int id) {
    // look up the mutex in the mutex table
    if (id < 0 || id >= MUTEX_MAX || !mutexes[id].allocated) return -1;
    // If the mutex is not locked, there is nothing to do
    if(mutexes[id].locks == 0)
        return 0;
    // Decrement the lock count
    --mutexes[id].locks;
    // If there are no more locks held:
    //    1. clear the owner of the mutex
    if(mutexes[id].locks == 0){
        mutexes[id].owner = NULL;
    }else{
        int* processID = NULL;
        proc_t* process;
        if(queue_out(&mutex_queue, processID)){
            process = pid_to_proc(*processID);
            scheduler_add(process);
            mutexes[id].owner = process;
        }
    }
    // If there are still locks held:
    //    1. Obtain a process from the mutex wait queue
    //    2. Add the process back to the scheduler
    //    3. set the owner of the of the mutex to the process

    // return the mutex lock count

    return mutexes[id].locks;
}

