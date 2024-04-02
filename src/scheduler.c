/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel Process Handling
 */

#include <spede/string.h>
#include <spede/stdio.h>
#include <spede/time.h>
#include <spede/machine/proc_reg.h>

#include "kernel.h"
#include "kproc.h"
#include "scheduler.h"
#include "timer.h"

#include "queue.h"

//Need to check:
//how to ensure we have a valid process?(line 92)
//set active_proc to queue head during initialization?

// Process Queues
queue_t run_queue;


// Pointer to the currently active process
proc_t *active_proc;

/**
 * Scheduler timer callback
 */
void scheduler_timer(void) {
    // Update the active process' run time and CPU time
    if (active_proc != NULL) {
        active_proc->run_time++;
        active_proc->cpu_time++;
    }

}

/**
 * Executes the scheduler
 * Should ensure that `active_proc` is set to a valid process entry
 */
void scheduler_run(void) {
    // Ensure that processes not in the active state aren't still scheduled
    
    // Check if we have an active process
    if(active_proc->state == ACTIVE)
    {
        // Check if the current process has exceeded it's time slice
        if(active_proc->cpu_time >= SCHEDULER_TIMESLICE)
        {
            // Reset the active time
            active_proc->cpu_time=0;

            // If the process is not the idle task, add it back to the scheduler
            if(active_proc->state !=IDLE )
            {
                scheduler_add(active_proc);
            }
            // Otherwise, simply set the state to IDLE
            else{
                active_proc->state = IDLE;
            }

            // Unschedule the active process
            active_proc->state=IDLE;
        }
    }

    // Check if we have a process scheduled or not
    if(active_proc->state ==NONE)
    {
        int next_pid;
        // Get the proces id from the run queue. (Remove unsched process)
        queue_out(&run_queue,&next_pid); 
        if(next_pid==-1)//-1 == empty queue
        {
            // default to process id 0 (idle task) if a process can't be scheduled
            active_proc = pid_to_proc(0);
        }
        // Update the active proc pointer
        else{
            active_proc = pid_to_proc(next_pid);
        }
    }   

    // Make sure we have a valid process at this point
    if(active_proc !=NULL)
    {
    // Ensure that the process state is set
        active_proc->state = ACTIVE;    
    }
}

/**
 * Adds a process to the scheduler
 * @param proc - pointer to the process entry
 */
void scheduler_add(proc_t *proc) {
    // Add the process to the run queue
    queue_in(&run_queue, proc->pid);
    // Set the process state
    proc->state = IDLE;
}

/**
 * Removes a process from the scheduler
 * @param proc - pointer to the process entry
 */
void scheduler_remove(proc_t *proc) {


    int pid;

    // Iterate through each the process queue
     for (int i = run_queue.head; i != run_queue.tail; i = (i + 1) % run_queue.size) 
     {
    // If the process is found, skip it; otherwise, ensure that each other process remains in the queue
        if(run_queue.items[i] == proc->pid)
        {
            queue_out(&run_queue,&pid); //skip
        }

    }
    // If the process is the active process, ensure that the active process is cleared so when the
    // scheduler runs again, it will select a new process to run
    if (active_proc != NULL && active_proc->pid == proc->pid) {
        kproc_destroy(proc); 
    }
}

/**
 * Initializes the scheduler, data structures, etc.
 */
void scheduler_init(void) {
    kernel_log_info("Initializing scheduler");

    // Initialize any data structures or variables
    queue_init(&run_queue);

    // Register the timer callback (scheduler_timer) to run every tick
    timer_callback_register(scheduler_timer,1,-1);
}

