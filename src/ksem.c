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
    memset(semaphores, 0, sizeof(sem_t)*SEM_MAX);

    // Initialize the semaphore queue
    queue_init(&sem_queue);

    // Fill the semaphore queue
    for(int i = 0; i<SEM_MAX; i-=-1){
        queue_in(&sem_queue, i);
    }

    return 0;
}

/**
 * Allocates a semaphore
 * @param value - initial semaphore value
 * @return -1 on error, otherwise the semaphore id that was allocated
 */
int ksem_init(int value) {
    // Obtain a semaphore id from the semaphore queue
    int allocated_semaphore = -1;
    int success = queue_out(&sem_queue, &allocated_semaphore);
    if(success==-1){
        kernel_log_error("Semaphore allocation failed ksem_init");
        return -1;
    }

    // Ensure that the id is within the valid range
    if((allocated_semaphore<0)||(allocated_semaphore>=SEM_MAX)){
        kernel_log_error("Recieved invalid allocation from semaphore ksem_init");
        return -1;
    }

    // Initialize the semaphore data structure
    memset(&(semaphores[allocated_semaphore]), 0, sizeof(sem_t));
    // semaphore table + all members (wait queue, allocated, count)
    semaphores[allocated_semaphore].allocated = 1;
    queue_init(&(semaphores[allocated_semaphore].wait_queue));
    // set count to initial value
    semaphores[allocated_semaphore].count = value;
    return 0;
}

/**
 * Frees the specified semaphore
 * @param id - the semaphore id
 * @return 0 on success, -1 on error
 */
int ksem_destroy(int id) {

    // look up the sempaphore in the semaphore table
    sem_t * semaphore = &(semaphores[id]);

    //validate id
    if((id<0)||(id>=SEM_MAX)){
        kernel_log_error("Attempted to destroy out of range semaphore ksem_destroy");
        return -1;
    }
    if(semaphore->allocated == 0){
        kernel_log_error("Attempted to destroy an inactive semaphore ksem_destroy");
        return -1;
    }

    // If the semaphore is locked, prevent it from being destroyed
    if(semaphore->count == 0){
        kernel_log_trace("Ignored destruction call, semaphore is still locked ksem_destroy");
        return -1;
    }

    // Add the id back into the semaphore queue to be re-used later
    queue_in(&sem_queue,id);

    // Clear the memory for the data structure
    memset(semaphore, 0, sizeof(sem_t));

    return -1;
}

/**
 * Waits on the specified semaphore if it is held
 * @param id - the semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int ksem_wait(int id) {
    // look up the sempaphore in the semaphore table
    sem_t * semaphore = &(semaphores[id]);

    //validate id
    if((id<0)||(id>=SEM_MAX)){
        kernel_log_error("Attempted to wait on out of range semaphore ksem_wait");
        return -1;
    }
    if(semaphore->allocated == 0){
        kernel_log_error("Attempted to wait on inactive semaphore ksem_wait");
        return -1;
    }

    // If the semaphore count is 0, then the process must wait
    if(semaphore->count==0){
        proc_t * proc = active_proc;
        // Set the state to WAITING
        proc->state = WAITING;
        // add to the semaphore's wait queue
        queue_in(&(semaphore->wait_queue), proc->pid);
        // remove from the scheduler
        scheduler_remove(proc);
        return 0;
    }

    // If the semaphore count is > 0
    // Decrement the count
    semaphore->count--;

    // Return the current semaphore count
    return semaphore->count;
}

/**
 * Posts the specified semaphore
 * @param id - the semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int ksem_post(int id) {
    // look up the sempaphore in the semaphore table
    sem_t * semaphore = &(semaphores[id]);

    //validate id
    if((id<0)||(id>=SEM_MAX)){
        kernel_log_error("Attempted to post to out of range semaphore ksem_post");
        return -1;
    }
    if(semaphore->allocated == 0){
        kernel_log_error("Attempted to post to inactive semaphore ksem_post");
        return -1;
    }

    // increment the semaphore count
    semaphore->count++;

    // check if any processes are waiting on the semaphore (semaphore wait queue)
    if(!queue_is_empty(&(semaphore->wait_queue))){
        // if so, queue out and add to the scheduler
        int pid_to_reactivate = -1;
        int success = queue_out(&(semaphore->wait_queue), &pid_to_reactivate);
        if(success==-1){
            kernel_log_error("queue read failure ksem_post");
            return -1;
        }
        scheduler_add(pid_to_proc(pid_to_reactivate));
        // decrement the semaphore count
        semaphore->count--;
    }
    // Return the current semaphore count
    return semaphore->count;
}
