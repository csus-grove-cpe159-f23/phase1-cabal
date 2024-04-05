/** //f
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel Process Handling
 */
//d

#include <spede/string.h> //f
#include <spede/stdio.h>
#include <spede/time.h>
#include <spede/machine/proc_reg.h>

#include "kernel.h"
#include "kproc.h"
#include "scheduler.h"
#include "timer.h"

#include "queue.h"
//d

// Process Queues
queue_t run_queue;

// Pointer to the currently active process
proc_t *active_proc;

void scheduler_timer(void) {//f
/**
 * Scheduler timer callback
 */
    // Update the active process' run time and CPU time
    if (active_proc != NULL) {
        active_proc->run_time++;
        active_proc->cpu_time++;
    }
}
//d
void scheduler_run(void) { //f
/**
 * Executes the scheduler
 * Should ensure that `active_proc` is set to a valid process entry
 */
    // Ensure that processes not in the active state aren't still scheduled
    kernel_log_info("scheduler run");

    // Check if we have an active process //f
    if(active_proc != NULL){
        // Check if the current process has exceeded it's time slice
        if(active_proc->cpu_time >= SCHEDULER_TIMESLICE){
            // Reset the active time
            active_proc->cpu_time = 0;

            // If the process is not the idle task, add it back to the scheduler
            if(active_proc->pid != 0){
                scheduler_add(active_proc);
            }
            // Otherwise, simply set the state to IDLE
            active_proc->state = IDLE;
            // Unschedule the active process
            active_proc = NULL;
        }
    }
    //d
    int next_pid = -1;
    // Check if we have a process scheduled or not
    if(active_proc == NULL){
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
    //f deal with paranoia
    if(active_proc == NULL){
        kernel_log_error("scheduler_run drew pid %d from the queue, this process no longer exists, breakpoint.",next_pid);//TODO this line may be unneccissary?
        kernel_break();
    }
    //d

    // Ensure that the process state is set
    active_proc->state = ACTIVE;
}
//d
void scheduler_add(proc_t *proc) { //f
/**
 * Adds a process to the scheduler
 * @param proc - pointer to the process entry
 */
    // Add the process to the run queue
    queue_in(&run_queue, proc->pid);
    // Set the process state
    proc->state = IDLE;
}
//d
void scheduler_remove(proc_t *proc) { //f
/**
 * Removes a process from the scheduler
 * @param proc - pointer to the process entry
 */
    // Iterate through each the process queue
     for (int i = 0; i < run_queue.size; i++) 
     {
        int queue_pid = -1;
        queue_out(&run_queue, &queue_pid);
        if(queue_pid == -1){kernel_panic("this probably shouldn't happen scheduler_remove");}
    // If the process is found, skip it; otherwise, ensure that each other process remains in the queue
        if(queue_pid != proc->pid) // only keep the ones that don't match lol
        {
            queue_in(&run_queue,queue_pid); // don't skip
        }
    }
    // If the process is the active process, ensure that the active process is cleared so when the
    // scheduler runs again, it will select a new process to run
    if (active_proc != NULL && active_proc->pid == proc->pid) {
        //kproc_destroy(proc);
        active_proc = NULL;
    }
}
//d
void scheduler_init(void) { //f
/**
 * Initializes the scheduler, data structures, etc.
 */
    kernel_log_info("Initializing scheduler");

    // Initialize any data structures or variables
    queue_init(&run_queue);

    // Register the timer callback (scheduler_timer) to run every tick
    timer_callback_register(scheduler_timer,1,-1);

    //char * idle = "idle";
    //kproc_create(kproc_idle, idle, PROC_TYPE_KERNEL);
    //kernel_log_info("scheduler run by hannah starting");
    //scheduler_run();// Hannah put this here because she thinks it needs to exist!
    //kernel_log_info("scheduler run by hannah done");
    //kernel_log_info("current active process name is %s it should be idle", active_process->name);
}
//d
