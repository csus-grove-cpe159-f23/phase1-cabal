/**
 * CPE/CSC 159 Operating System Pragmatics
 * California State University, Sacramento
 *
 * Operating system entry point
 */

#include "vga.h"
#include "keyboard.h"
#include "bit.h"
#include "interrupts.h"
#include "timer.h"
#include "tty.h"

static int my_int = 0;
void test_print(void){
    printf("test_print %d\n", my_int);
    my_int++;
}

void main(void) {

    //initialize drivers
    vga_init();
    interrupts_init();
    keyboard_init();
    timer_init();
    tty_init();
    vga_printf("Welcome to %s!\n", OS_NAME);
    keyboard_getc();
    interrupts_enable();
    timer_callback_register(test_print, 300, 10);
    // Loop in place forever
    while (1) {
        char c = keyboard_poll();
        if (c) {
            tty_update(c);
        }
    }
    // We should never get here!
}
