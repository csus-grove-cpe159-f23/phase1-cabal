/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel System Call Handlers
 */
#include <spede/time.h>
#include <spede/string.h>
#include <spede/stdio.h>

#include "kernel.h"
#include "kproc.h"
#include "ksyscall.h"
#include "interrupts.h"
#include "scheduler.h"
#include "timer.h"
#include "ksem.h"
#include "kmutex.h"

/**
 * System call IRQ handler
 * Dispatches system calls to the function associate with the specified system call
 */
void ksyscall_irq_handler(void) {
    // Default return value
    int rc = -1;

    if (!active_proc) {
        kernel_panic("Invalid process");
    }

    if (!active_proc->trapframe) {
        kernel_panic("Invalid trapframe");
    }

    // System call identifier is stored on the EAX register
    // Additional arguments should be stored on additional registers (EBX, ECX, etc.)

    // Based upon the system call identifier, call the respective system call handler

    // Ensure that the EAX register for the active process contains the return value
    int syscall = active_proc->trapframe->eax;
    int arg1 = active_proc->trapframe->ebx;
    int arg2 = active_proc->trapframe->ecx;
    int arg3 = active_proc->trapframe->edx;
//    kernel_log_info("syscall made %x", syscall);
    proc_t * proc = active_proc;

    switch(syscall){
    case SYSCALL_PROC_GET_PID:
        rc = ksyscall_proc_get_pid();
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_PROC_GET_NAME:
        rc = ksyscall_proc_get_name((char*)arg1);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_PROC_EXIT:
        ksyscall_proc_exit();
        return;
    case SYSCALL_IO_WRITE:
        rc = ksyscall_io_write(arg1,(char*)arg2, arg3);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_IO_READ:
        rc = ksyscall_io_read(arg1, (char*)arg2, arg3);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_IO_FLUSH:
        rc = ksyscall_io_flush(arg1);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_PROC_SLEEP:
        rc = ksyscall_proc_sleep(arg1);
        return;
    case SYSCALL_MUTEX_INIT:
        rc = ksyscall_mutex_init();
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_MUTEX_DESTROY:
        rc = ksyscall_mutex_destroy(arg1);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_MUTEX_LOCK:
        rc = ksyscall_mutex_lock(arg1);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_MUTEX_UNLOCK:
        rc = ksyscall_mutex_unlock(arg1);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_SEM_INIT:
        rc = ksyscall_sem_init(arg1);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_SEM_DESTROY:
        rc = ksyscall_sem_destroy(arg1);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_SEM_WAIT:
        rc = ksyscall_sem_wait(arg1);
        proc->trapframe->eax = rc;
        return;
    case SYSCALL_SEM_POST:
        rc = ksyscall_sem_post(arg1);
        proc->trapframe->eax = rc;
        return;
    }

    if (proc->trapframe->eax == SYSCALL_SYS_GET_TIME) {
        rc = ksyscall_sys_get_time();
        proc->trapframe->eax = rc;
        return;
    }

    if (proc->trapframe->eax == SYSCALL_SYS_GET_NAME) {
        // Cast the argument as a char pointer
        rc = ksyscall_sys_get_name((char *)proc->trapframe->ebx);
        proc->trapframe->eax = rc;
        return;
    }

    kernel_panic("Invalid system call %d!", active_proc->trapframe->eax);
}

/**
 * System Call Initialization
 */
void ksyscall_init(void) {
    interrupts_irq_register(IRQ_SYSCALL, isr_entry_syscall, ksyscall_irq_handler);
    // Register the IDT entry and IRQ handler for the syscall IRQ (IRQ_SYSCALL)
}

/**
 * Writes up to n bytes to the process' specified IO buffer
 * @param io - the IO buffer to write to
 * @param buf - the buffer to copy from
 * @param n - number of bytes to write
 * @return -1 on error or value indicating number of bytes copied
 */
int ksyscall_io_write(int io, char *buf, int size) {

    // Ensure there is an active process
    if(!active_proc)
        kernel_panic("No active process!");
    // Ensure the IO buffer is withing range (PROC_IO_MAX)
    if((io > PROC_IO_MAX)||(io < 0)){
        kernel_log_error("Out of range IO buffer specified, ksyscall_io_write");
        return -1;
    }
    // Ensure that the active process has valid io
    // If not active_proc->....
    if(active_proc->io[io]){
        ringbuf_write_mem(active_proc->io[io], buf, size);
        return size;
    }
    // Using ringbuf_write_mem - Write size bytes from buf to active_proc->io...
    return -1;
}

/**
 * Reads up to n bytes from the process' specified IO buffer
 * @param io - the IO buffer to read from
 * @param buf - the buffer to copy to
 * @param n - number of bytes to read
 * @return -1 on error or value indicating number of bytes copied
 */
int ksyscall_io_read(int io, char *buf, int size) {
    if(!active_proc)
        kernel_panic("No active process!");
    // Ensure there is active process, IO buffer is within range, active process has valid io
    if(!active_proc->io[io])
        return -1;
    if(io <= PROC_IO_MAX){
        int read_length = ringbuf_read_mem(active_proc->io[io], buf, size);
        return read_length;
    }
    // Using ringbuf_read_mem - Read size bytes from active_proc->io to buf
    return -1;
}

/**
 * Flushes (clears) the specified IO buffer
 * @param io - the IO buffer to flush
 * @return -1 on error or 0 on success
 */
int ksyscall_io_flush(int io) {
    if(!active_proc)
        kernel_panic("No active process!");
    // Ensure active process, etc...
    if((io > PROC_IO_MAX)||(io < 0)){
        kernel_log_error("Out of range IO buffer specified, ksyscall_io_flush");
        return -1;
    }
    if(active_proc->io[io]){
        ringbuf_flush(active_proc->io[io]);
        return 0;
    }
    // Use ringbuf_flush to flush io buffer
    return -1;
}

/**
 * Gets the current system time (in seconds)
 * @return system time in seconds
 */
int ksyscall_sys_get_time(void) {
    return timer_get_ticks() / 100;
}

/**
 * Gets the operating system name
 * @param name - pointer to a character buffer where the name will be copied
 * @return 0 on success, -1 or other non-zero value on error
 */
int ksyscall_sys_get_name(char *name) {
    if (!name) {
        return -1;
    }

    strncpy(name, OS_NAME, sizeof(OS_NAME));
    return 0;
}

/**
 * Puts the active process to sleep for the specified number of seconds
 * @param seconds - number of seconds the process should sleep
 */
int ksyscall_proc_sleep(int seconds) {
    if(active_proc){
        scheduler_sleep(active_proc, seconds);
    }else{
        kernel_log_error("sleep syscall initiated while there is no active process? ksyscall_proc_sleep");
    }
    return 0; // honestly I'm not sure what I should return. Zero should be ok? -Hannah
}

/**
 * Exits the current process
 */
int ksyscall_proc_exit(void) {
    if(kproc_destroy(active_proc))
        return 0;
    return -1;
}

/**
 * Gets the active process pid
 * @return process id or -1 on error
 */
int ksyscall_proc_get_pid(void) {
    if(!active_proc->pid)
        return -1;
    return active_proc->pid;
}

/**
 * Gets the active process' name
 * @param name - pointer to a character buffer where the name will be copied
 * @return 0 on success, -1 or other non-zero value on error
 */
int ksyscall_proc_get_name(char *name) {
    if(!name)
        return -1;
    strncpy(name, active_proc->name, sizeof(active_proc->name));
    return 0;
}

/**
 * Allocates a semaphore from the kernel
 * @param value - initial semaphore value
 * @return -1 on error, all other values indicate the semaphore id
 */
int ksyscall_sem_init(int value){
    return ksem_init(value);
}

/**
 * Destroys a semaphore
 * @param sem - semaphore id
 * @return -1 on error, 0 on success
 */
int ksyscall_sem_destroy(int sem){
    return ksem_destroy(sem);
}

/**
 * Waits on a semaphore
 * @param sem - semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int ksyscall_sem_wait(int sem){
    return ksem_wait(sem);
}

/**
 * Posts a semaphore
 * @param sem - semaphore id
 * @return -1 on error, otherwise the current semaphore count
 */
int ksyscall_sem_post(int sem){
    return ksem_post(sem);
}


/**
 * Allocates a mutex from the kernel
 * @return -1 on error, all other values indicate the mutex id
 */
int ksyscall_mutex_init(void) {
    return kmutex_init();
}

/**
 * Detroys a mutex
 * @return -1 on error, 0 on sucecss
 */
int ksyscall_mutex_destroy(int mutex) {
    return kmutex_destroy(mutex);
}

/**
 * Locks the mutex
 * @param mutex - mutex id
 * @return -1 on error, 0 on sucecss
 * @note If the mutex is already locked, process will block/wait.
 */
int ksyscall_mutex_lock(int mutex) {
    return kmutex_lock(mutex);
}

/**
 * Unlocks the mutex
 * @param mutex - mutex id
 * @return -1 on error, 0 on sucecss
 */
int ksyscall_mutex_unlock(int mutex) {
    return kmutex_unlock(mutex);
}

