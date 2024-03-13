/**
 * CPE/CSC 159 Operating System Pragmatics
 * California State University, Sacramento
 *
 * Operating system entry point
 */

#include "interrupts.h"
#include "kernel.h"
#include "keyboard.h"
#include "timer.h"
#include "tty.h"
#include "vga.h"

#include "test.h"

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
    while (1) {
        char c = keyboard_poll();
        char buf[80] = {0};

        if (c) {
            // Print the character on the screen
            vga_putc(c);

            // Copy the current x/y (cursor) position to a buffer
            snprintf(buf, sizeof(buf), "%02d, %02d", vga_get_y(), vga_get_x());

            // Print the buffer to the screen
            vga_puts_at(VGA_HEIGHT-1, 74, VGA_COLOR_LIGHT_RED, VGA_COLOR_WHITE,
                        buf);
            
            /*if(vga_get_y()>= 25)//if at end of page
                {
                    tty_scroll_down();
                    vga_clear();
                }*/
        }
    }

    // Should never get here!
    return 0;
}
