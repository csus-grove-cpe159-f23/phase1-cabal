/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Keyboard Functions
 */
#include "io.h"
#include "kernel.h"
#include "keyboard.h"

/**
 * Initializes keyboard data structures and variables
 */
void keyboard_init() {
    kernel_log_info("Initializing keyboard driver");
}

/**
 * Scans for keyboard input and returns the raw character data
 * @return raw character data from the keyboard
 */
unsigned int keyboard_scan(void) {
    unsigned int c = KEY_NULL;
    return c;
}

/**
 * Polls for a keyboard character to be entered.
 *
 * If a keyboard character data is present, will scan and return
 * the decoded keyboard output.
 *
 * @return decoded character or KEY_NULL (0) for any character
 *         that cannot be decoded
 */
unsigned int keyboard_poll(void) {
    unsigned int c = KEY_NULL;
    return c;
}

/**
 * Blocks until a keyboard character has been entered
 * @return decoded character entered by the keyboard or KEY_NULL
 *         for any character that cannot be decoded
 */
unsigned int keyboard_getc(void) {
    unsigned int c = KEY_NULL;
    while ((c = keyboard_poll()) == KEY_NULL);
    return c;
}

/**
 * Processes raw keyboard input and decodes it.
 *
 * Should keep track of the keyboard status for the following keys:
 *   SHIFT, CTRL, ALT, CAPS, NUMLOCK
 *
 * For all other characters, they should be decoded/mapped to ASCII
 * or ASCII-friendly characters.
 *
 * For any character that cannot be mapped, KEY_NULL should be returned.
 *
 * If *all* of the status keys defined in KEY_KERNEL_DEBUG are pressed,
 * while another character is entered, the kernel_debug_command()
 * function should be called.
 */
unsigned int keyboard_decode(unsigned int c) {
    return c;
}
