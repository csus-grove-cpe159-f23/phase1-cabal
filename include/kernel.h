/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel Functions and Helpers
 */
#ifndef KERNEL_H
#define KERNEL_H

#define KSTACK_SIZE 16384
#define KCODE_SEG 0x08
#define KDATA_SEG 0x10

#ifndef ASSEMBLER
#include "kproc.h"
#include <spede/machine/asmacros.h>

#ifndef OS_NAME
#define OS_NAME "MyOS"
#endif

// List of kernel log levels in order of severity
typedef enum log_level {
    KERNEL_LOG_LEVEL_NONE,  // No Logging!
    KERNEL_LOG_LEVEL_ERROR, // Log only errors
    KERNEL_LOG_LEVEL_WARN,  // Log warnings and errors
    KERNEL_LOG_LEVEL_INFO,  // Log info, warnings, and errors
    KERNEL_LOG_LEVEL_DEBUG, // Log debug, info, warnings, and errors
    KERNEL_LOG_LEVEL_TRACE, // Log trace, debug, info, warnings, and errors
    KERNEL_LOG_LEVEL_ALL    // Log everything!
} log_level_t;

//global pointer to the current active process.
extern proc_t *active_proc;

/**
 * Kernel initialization
 *
 * Initializes any kernel/global data structures and variables
 */
void kernel_init(void);

/**
 * Function declarations
 */

/**
 * Prints a kernel log message to the host with an error log level
 *
 * @param msg - string format for the message to be displayed
 * @param ... - variable arguments to pass in to the string format
 */
void kernel_log_error(char *msg, ...);

/**
 * Prints a kernel log message to the host with a warning log level
 *
 * @param msg - string format for the message to be displayed
 * @param ... - variable arguments to pass in to the string format
 */
void kernel_log_warn(char *msg, ...);

/**
 * Prints a kernel log message to the host with an info log level
 *
 * @param msg - string format for the message to be displayed
 * @param ... - variable arguments to pass in to the string format
 */
void kernel_log_info(char *msg, ...);

/**
 * Prints a kernel log message to the host with a debug log level
 *
 * @param msg - string format for the message to be displayed
 * @param ... - variable arguments to pass in to the string format
 */
void kernel_log_debug(char *msg, ...);

/**
 * Prints a kernel log message to the host with a trace log level
 *
 * @param msg - string format for the message to be displayed
 * @param ... - variable arguments to pass in to the string format
 */
void kernel_log_trace(char *msg, ...);

/**
 * Triggers a kernel panic that does the following:
 *   - Displays a panic message on the host console
 *   - Triggers a breakpiont (if running through GDB)
 *   - aborts/exits the operating system program
 *
 * @param msg - string format for the message to be displayed
 * @param ... - variable arguments to pass in to the string format
 */
void kernel_panic(char *msg, ...);

/**
 * Returns the current log level
 * @return the kernel log level
 */
int kernel_get_log_level(void);

/**
 * Sets the new log level and returns the value set
 * @param level - the log level to set
 * @return the kernel log level
 */
int kernel_set_log_level(log_level_t level);

/**
 * Triggers a breakpoint (if running under GDB)
 */
void kernel_break(void);

/**
 * Exits the kernel
 */
void kernel_exit(void);

void kernel_context_enter(trapframe_t *trapframe);

//Following functions are written irectly in assembly.
__BEGIN_DECLS
//Exits the kernel context, restoring the process.
extern void kernel_context_exit();
__END_DECLS
#endif
#endif