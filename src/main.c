/**
 * CPE/CSC 159 Operating System Pragmatics
 * California State University, Sacramento
 *
 * Operating system entry point
 */

#include <spede/stdbool.h>
#include "interrupts.h"
#include "kernel.h"
#include "keyboard.h"
#include "timer.h"
#include "tty.h"
#include "vga.h"
#include "scheduler.h"
#include "kproc.h"
#include "test.h"
#include "ksyscall.h"
#include "kmutex.h"
#include "ksem.h"

int main(void) {
    // Always iniialize the kernel
    kernel_init();

    // Initialize interrupts
    interrupts_init();

    // Initialize timers
    timer_init();

    // Initialize the TTY
    tty_init();

    // Initialize the VGA driver
    vga_init();

    // Initialize the keyboard driver
    keyboard_init();

    // Initialize the scheduler
    scheduler_init();

    //THIS NEEDS TO BE AFTER THE SCHEDULER - Hannah
    // Initialize processes
    kproc_init();
    
    ksyscall_init();

    ksemaphores_init();

    kmutexes_init();

    // Test initialization
    test_init();

    // Print a welcome message
    vga_printf("Welcome to %s!\n", OS_NAME);
    vga_puts("Press a key to continue...\n");

    // Wait for a key to be pressed
    keyboard_getc();

    // Clear the screen
    vga_clear();

    // Enable interrupts
    interrupts_enable();

    // Loop in place forever
    while (1);

    // Should never get here!
    return 0;
}
