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
queue_t sleep_queue;

// Pointer to the currently active process
proc_t *active_proc;

void scheduler_timer(void) {//f
/**
 * Scheduler timer callback
 */
    // Update the active process' run time and CPU time
    // TODO update this for the sleep thing?
    if (active_proc != NULL) {
        active_proc->run_time++;
        active_proc->cpu_time++;
    }
    for(int i = 0; i < sleep_queue.size; i++){
        int pid = -1;
        int success = queue_out(&sleep_queue,&pid);
        if(success==-1){kernel_log_error("Bad queue read schedule_timer");}
        proc_t* proc = pid_to_proc(pid);
        proc->sleep_time--;
        if(proc->sleep_time<=0){
            kernel_log_info("process pid: %d finished sleeping", pid);
            queue_in(&run_queue, pid);
        }else{
            queue_in(&sleep_queue, pid);
        }
    }
}
//d
void scheduler_run(void) { //f
/**
 * Executes the scheduler
 * Should ensure that `active_proc` is set to a valid process entry
 */
    // Ensure that processes not in the active state aren't still scheduled
    //kernel_log_info("scheduler run");

    // Check if we have an active process //f
    if(active_proc != NULL){
        // Check if the current process has exceeded it's time slice
        if(active_proc->cpu_time >= SCHEDULER_TIMESLICE){
            // Reset the active time
            active_proc->cpu_time = 0;

            // If the process is not the idle task, add it back to the scheduler
            if(active_proc->pid != 0){
                queue_in(&run_queue, active_proc->pid);
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
int remove_item_from_queue(queue_t * removal_queue,int desired_item){ //f
    // loops through a queue searching for an specific item. returns 1 if found, 0 if not found.
    // I am iritated that queue is the data structure specified by specification. This feels not right.
    for(int i = 0; i< removal_queue->size; i++){
        int current_item = -1;
        int success = queue_out(removal_queue, &current_item);
        if(current_item == -1){kernel_panic("How the fudgesickles did a -1 get in the PID queue? remove_item_from_queue");}
        if(success == -1){kernel_panic("Queue read failed! remove_item_from_queue");}
        if(current_item == desired_item){
            return 1;
        }
        queue_in(removal_queue, current_item);
    }
    return 0;
}
//d
void scheduler_remove(proc_t *proc) { //f
/**
 * Removes a process from the scheduler
 * @param proc - pointer to the process entry
 */
    remove_item_from_queue(&run_queue, proc->pid);
    remove_item_from_queue(&sleep_queue, proc->pid);
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
    queue_init(&sleep_queue);

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
void scheduler_sleep(proc_t *proc, int seconds){ //f
/** //f
 * Puts a process to sleep
 * @param proc - pointer to the process entry
 * @param seconds - number of seconds to sleep
 */
//d
    proc->sleep_time = 100*seconds;
    proc->state = SLEEPING;
    if(remove_item_from_queue(&run_queue, proc->pid)){
        // if we find our item in the run queue, move it to the sleep queue
        queue_in(&sleep_queue, proc->pid);
        return;
    }
    if((active_proc)&&(active_proc==proc)){
        // if our process was the current active process, make it not the active process. Active processes can't be asleep!
        queue_in(&sleep_queue, proc->pid);
        active_proc=NULL;
        return;
    }
    if(remove_item_from_queue(&sleep_queue, proc->pid)){
        //out process was already sleeping? cool I guess? just keep sleeping
        queue_in(&sleep_queue, proc->pid);
        return;
    }
    //our our process is not one of the scheduled processes???? whoops? scream an error
    kernel_log_error("scheduler instructed to sleep process which is not actively scheduled. This behavior is unspecified.");
}
//d
