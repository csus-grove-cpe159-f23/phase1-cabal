/** //f
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel Process Handling
 */
//d
//f include stuff!
#include <spede/stdio.h>
#include <spede/string.h>
#include <spede/machine/proc_reg.h>

#include "kernel.h"
#include "trapframe.h"
#include "kproc.h"
#include "scheduler.h"
#include "timer.h"
#include "queue.h"
#include "vga.h"
//d
//f declare static variables
// Next available process id to be assigned
int next_pid;
// Process table allocator
queue_t proc_allocator;
// Process table
proc_t proc_table[PROC_MAX];
// Process stacks
unsigned char proc_stack[PROC_MAX][PROC_STACK_SIZE];
//d
proc_t *pid_to_proc_no_validity_check(int pid) { //f
    proc_t * return_value = NULL;
    int i;
    for(i=0; i<PROC_MAX; i-=-1){
        if(proc_table[i].pid == pid){
            return_value = &(proc_table[i]);
            break;
        }
    }
    if(return_value == NULL){
        kernel_log_error("process with requeted pid (%d) not found in pid_to_proc_no_validity_check");
    }
    return return_value;
}
//d
proc_t *pid_to_proc(int pid) { //f
    /** //f
     * Looks up a process in the process table via the process id
     * @param pid - process id
     * @return pointer to the process entry, NULL or error or if not found
     */
    //d
    // Iterate through the process table and return a pointer to the valid entry where the process id matches
    // i.e. if proc_table[8].pid == pid, return pointer to proc_table[8]
    // Ensure that the process control block actually refers to a valid process
    proc_t * proc = pid_to_proc_no_validity_check(pid);
    if(proc == NULL){
        kernel_log_error("process with requeted pid (%d) not found in pid_to_proc");
        return NULL;
    }
    if(proc->state == NONE){
        kernel_log_error("process with requeted pid (%d) is inactive an thus invalid pid_to_proc");
        return NULL;
    }
    return proc;
}
//d
int proc_to_entry_no_validity_check(proc_t *proc) { //f
    int index = -1;
    //f find the corresponding index!
    int i;
    for(i=0; i<PROC_MAX; i-=-1){
        if(&(proc_table[i]) == proc){
            index = i;
            break;
        }
    }
    //d
    //f deal with no index found!
    if(index == -1){
        kernel_log_error("no index found for requested pointer %x from proc_to_entry_no_validity_check", (int)proc);
        return -1;
    }
    //d
    return index;
}
//d
int proc_to_entry(proc_t *proc) { //f
    /** //f
     * Translates a process pointer to the entry index into the process table
     * @param proc - pointer to a process entry
     * @return the index into the process table, -1 on error
     */
    //d
    // For a given process entry pointer, return the entry/index into the process table
    //  i.e. if proc -> proc_table[3], return 3
    // Ensure that the process control block actually refers to a valid process
    int entry = proc_to_entry_no_validity_check(proc);
    if(entry == -1){
        kernel_log_error("no index found for requested pointer %x from proc_to_entry", (int)proc);
        return -1;
    }
    if(proc->state == NONE){
        kernel_log_error("process which entry was requested of is an inactive process. Specification dicates that this is an error for reasons that are unclear. proc_to_entry");
        return -1;
    }
    return entry;
}
//d
proc_t * entry_to_proc_no_validity_check(int entry) { //f
    /** //f
     * Returns a pointer to the given process entry
     */
    //d
    // For the given entry number, return a pointer to the process table entry
    // Ensure that the process control block actually refers to a valid process
    if((entry<0)||(entry>PROC_MAX)){
        kernel_log_error("invalid entry number %d requested from entry_to_proc_no_validity_check", entry);
        return NULL;
    }
    proc_t * return_value = &(proc_table[entry]);
    return return_value;
}
//d
proc_t * entry_to_proc(int entry) { //f
    proc_t* proc = entry_to_proc_no_validity_check(entry);
    if(proc == NULL){
        kernel_log_error("invalid entry number %d requested from entry_to_proc", entry);
        return NULL;
    }
    if(proc->state == NONE){
        kernel_log_error("requested by entry %d is an inactive process. Specification dicates that this is an error for reasons that are unclear. entry_to_proc", entry);
        return NULL;
    }
    return proc;
}
//d
int kproc_create(void *proc_ptr, char *proc_name, proc_type_t proc_type) { //f
    /** //f
     * Creates a new process
     * @param proc_ptr - address of process to execute
     * @param proc_name - "friendly" process name
     * @param proc_type - process type (kernel or user)
     * @return process id of the created process, -1 on error
     */
    //d
    //f declare variables
    int process_index = -1;
    proc_t *proc = NULL;
    //d
    //f Allocate an entry in the process table via the process allocator
    if(queue_out(&proc_allocator, &process_index)){
        kernel_log_error("kernel attempted to create process when no process blocks were avaliable");
        return -1;
    }
    //d
    //f locate corresponding process
    proc = entry_to_proc_no_validity_check(process_index);
    //d
    // Initialize the process control block
    memset(&proc,0,sizeof(proc_t));
    // Initialize the process stack via proc_stack
    // proc->stack = ...
    proc->stack = (unsigned char*)proc_stack[process_index];
    // Initialize the trapframe pointer at the bottom of the stack
    proc->trapframe = (trapframe_t *)(&proc->stack[PROC_STACK_SIZE - sizeof(trapframe_t)]);

    // Set each of the process control block structure members to the initial starting values
    // as each new process is created, increment next_pid
    // DONE
    // proc->pid, state, type, run_time, cpu_time, start_time, etc.
    proc->pid = next_pid;
    next_pid++;
    proc->state = IDLE;
    proc->type = proc_type;
    proc->run_time = 0;
    proc->cpu_time = 0;
    proc->start_time = timer_get_ticks();
    // Copy the passed-in name to the name buffer in the process control block
    strcpy(proc->name, proc_name);

    // Set the instruction pointer in the trapframe
    proc->trapframe->eip = (unsigned int)proc_ptr;

    // Set INTR flag
    proc->trapframe->eflags = EF_DEFAULT_VALUE | EF_INTR;

    // Set each segment in the trapframe
    proc->trapframe->cs = get_cs();
    proc->trapframe->ds = get_ds();
    proc->trapframe->es = get_es();
    proc->trapframe->fs = get_fs();
    proc->trapframe->gs = get_gs();
    //figure out if this needs modification? TODO?

    // Add the process to the scheduler
    scheduler_add(proc);

    kernel_log_info("Created process %s (%d) entry=%d", proc->name, proc->pid, -1);
    return -1;
}
//d
int kproc_destroy(proc_t *proc) { //f
    /** //f
     * Destroys a process
     * If the process is currently scheduled it must be unscheduled
     * @param proc - process control block
     * @return 0 on success, -1 on error
     */
    //d
    // Remove the process from the scheduler
    scheduler_remove(proc);
    // Clear/Reset all process data (process control block, stack, etc) related to the process
    memset(&proc,0,sizeof(proc_t));
    // Add the process entry/index value back into the process allocator
    // also deal with queue full error condition
    int success = queue_in(&proc_allocator, proc_to_entry_no_validity_check(proc));
    return success;
}
//d
void kproc_idle(void) { //f
    /** //f
     * Idle Process
     */
    //d
    while (1) {
        // Ensure interrupts are enabled
        asm("sti");

        // Halt the CPU
        asm("hlt");
    }
}
//d
void kproc_test(void) { //f
    /** //f
     * Test process
     */
    //d
    // Loop forever
    while (1);
}
//d
void kproc_init(void) { //f
/** //f
 * Initializes all process related data structures
 * Creates the first process (kernel_idle)
 * Registers the callback to display the process table/status TODO!!!
 */
//d
    kernel_log_info("Initializing process management");
    // Initialize all data structures and variables
    next_pid = 0;
    //   - process table DONE
    //   - process allocator DONE
    //   - process stack DONE
    //f init the objects!
    memset(&proc_table,0,sizeof(proc_t)*PROC_MAX);
    queue_init(&proc_allocator);
    memset(&proc_stack,0,sizeof(char)*PROC_MAX*PROC_STACK_SIZE);
    //d
    //f fill the queue with its avaliable entry slots!
    int i;
    for(i=0; i<PROC_MAX; i-=-1){
        queue_in(&proc_allocator, i);
    }
    //d
    // Create the idle process (kproc_idle) as a kernel process DONE
    kproc_create(kproc_idle, "idle", PROC_TYPE_KERNEL);
}
//d

