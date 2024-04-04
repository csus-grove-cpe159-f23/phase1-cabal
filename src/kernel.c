/** //f
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel functions
 */
//d
//f include stuff
#include <spede/flames.h>   // for breakpoint()
#include <spede/stdarg.h>   // for variable argument functions (va_*)
#include <spede/stdio.h>    // for printf
#include <spede/string.h>   // string handling

#include "kernel.h"
#include "vga.h"
#include "keyboard.h"
#include "scheduler.h"
#include "interrupts.h"
//d
//f set detault log level
// this feels like it should be in the header but ok
#ifndef KERNEL_LOG_LEVEL_DEFAULT
#define KERNEL_LOG_LEVEL_DEFAULT KERNEL_LOG_LEVEL_TRACE
#endif
//d
//f current log level global valriable
int kernel_log_level = KERNEL_LOG_LEVEL_DEFAULT;
//d

void kernel_init(void) { //f
    /**
     * Initializes any kernel internal data structures and variables
     */
    // Display a welcome message on the host
    kernel_log_info("Welcome to %s!", OS_NAME);
    kernel_log_info("Initializing kernel...");
}
//d
//f log_level_function_body macro 
#define log_level_function_body(__LEVEL__NUMBER__, __LEVEL__PRINT__STRING__) {\
    if (kernel_log_level < __LEVEL__NUMBER__) {\
        return;\
    }\
    va_list args;\
    printf(__LEVEL__PRINT__STRING__);\
    va_start(args, msg);vprintf(msg, args);va_end(args);\
    printf("\n");\
}
//d
void kernel_log_error(char *msg, ...) { //f
    /** //f
     * Prints a kernel log message to the host with an error log level
     *
     * @param msg - string format for the message to be displayed
     * @param ... - variable arguments to pass in to the string format
     */
    //d
    log_level_function_body(KERNEL_LOG_LEVEL_ERROR,"error: ")
}
//d 
void kernel_log_warn(char *msg, ...) { //f
    /** //f
     * Prints a kernel log message to the host with a warning log level
     *
     * @param msg - string format for the message to be displayed
     * @param ... - variable arguments to pass in to the string format
     */
    //d
    log_level_function_body(KERNEL_LOG_LEVEL_WARN,"warning: ")
}
//d
void kernel_log_info(char *msg, ...) { //f
    /**
     * Prints a kernel log message to the host with an info log level
     *
     * @param msg - string format for the message to be displayed
     * @param ... - variable arguments to pass in to the string format
     */
    // Return if our log level is less than info
    if (kernel_log_level < KERNEL_LOG_LEVEL_INFO) {
        return;
    }

    // Obtain the list of variable arguments
    va_list args;

    // Indicate this is an 'info' type of message
    printf("info: ");

    // Pass the message variable arguments to vprintf
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
    printf("\n");
}
//d
void kernel_log_debug(char *msg, ...) { //f
    /** //f
     * Prints a kernel log message to the host with a debug log level
     *
     * @param msg - string format for the message to be displayed
     * @param ... - variable arguments to pass in to the string format
     */
    //d
    log_level_function_body(KERNEL_LOG_LEVEL_DEBUG,"debug: ")
}
//d
void kernel_log_trace(char *msg, ...) { //f
    /** //f
     * Prints a kernel log message to the host with a trace log level
     *
     * @param msg - string format for the message to be displayed
     * @param ... - variable arguments to pass in to the string format
     */
    //d
    log_level_function_body(KERNEL_LOG_LEVEL_TRACE,"trace: ")
}
//d
void kernel_panic(char *msg, ...) { //f
    /** //f
     * Triggers a kernel panic that does the following:
     *   - Displays a panic message on the host console
     *   - Triggers a breakpiont (if running through GDB)
     *   - aborts/exits the operating system program
     *
     * @param msg - string format for the message to be displayed
     * @param ... - variable arguments to pass in to the string format
     */
    //d
    // Obtain the list of variable arguments
    va_list args;
    // Indicate this is an 'info' type of message
    printf("panic: ");
    vga_printf("panic: ");
    // Pass the message variable arguments to vprintf
    va_start(args, msg);
    vprintf(msg, args);
    vga_printf(msg, args);
    vprintf(msg, args);
    va_end(args);
    printf("\n");
    // Trigger a breakpoint to inspect what caused the panic
    kernel_break();
    // Exit since this is fatal
    exit(1);
}
//d
void kernel_context_enter(trapframe_t *trapframe){ //f
    //called to deal with every possible interrupt
    //f save the previous process
    if(active_proc){
        active_proc->trapframe = trapframe;
    }
    //d
    //f handle the interrupt
    interrupts_irq_handler(trapframe->interrupt);
    //d
    //f decide what to run next
    scheduler_run();
    //d
    //f deal with there being no processes to run
    if(!active_proc){
        kernel_panic("scheduler could not find process to run, not even the idle process!");
    }
    //d
    //f run the process that was selected
    kernel_context_exit();
    //d
}
//d
int kernel_get_log_level(void) { //f
    /**
     * Returns the current log level
     * @return the kernel log level
     */
    return kernel_log_level;
}
//d
int kernel_set_log_level(log_level_t level) { //f
    /**
     * Sets the new log level and returns the value set
     * @param level - the log level to set
     * @return the kernel log level
     */
    int prev_log_level = kernel_log_level;

    if (level < KERNEL_LOG_LEVEL_NONE) {
        kernel_log_level = KERNEL_LOG_LEVEL_NONE;
    } else if (level > KERNEL_LOG_LEVEL_ALL) {
        kernel_log_level = KERNEL_LOG_LEVEL_ALL;
    } else {
        kernel_log_level = level;
    }

    if (prev_log_level != kernel_log_level) {
        printf("<<kernel log level set to %d>>\n", kernel_log_level);
    }

    return kernel_log_level;
}
//d
void kernel_break(void) { //f
    /**
     * Triggers a breakpoint (if running under GBD)
     */
    breakpoint();
}
//d
void kernel_exit(void) { //f
    /**
     * Exits the kernel
     */
    // Print to the terminal
    printf("Exiting %s...\n", OS_NAME);
    // Print to the VGA display
    vga_printf("Exiting %s...\n", OS_NAME);
    // Exit
    exit(0);
}
//d

