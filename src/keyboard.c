
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
=======
/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Keyboard Functions
 */
#include "io.h"
#include "kernel.h"
#include "keyboard.h"

#define KEY_PRESSED(c) ((c & 0x80) == 0)
#define KEY_RELEASED(c) ((c & 0x80) != 0)

/*static const char keyboard_map_primary[] = {
   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'
};*/

//static const char keyboard_map_secondary[]

static int shiftState;
static int capslock;

/**
 * Initializes keyboard data structures and variables
 */
void keyboard_init() {
    kernel_log_info("Initializing keyboard driver");
    shiftState = 0;
    capslock = 0;
}

/**
 * Scans for keyboard input and returns the raw character data
 * @return raw character data from the keyboard
 */
unsigned int keyboard_scan(void) {
    unsigned int c = KEY_NULL;
    c = inportb(KBD_PORT_DATA);
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
    int status = inportb(KBD_PORT_STAT);
    int data = KEY_NULL;
    int ascii = KEY_NULL;
    if(status & 1){
        data = keyboard_scan();
        if(data == 0x2a || 0x36){
                shiftState =  0;
                if(data & 0x80)
                    shiftState = 1;
        }
        if(data == 0x3a && data & 0x80){
            if(capslock == 1){
                capslock = 0;
            }else{
                capslock = 1;
            }
        }
        ascii = keyboard_decode(data);
        kernel_log_info("Key Pressed. status: %4d Keycode: %5d ascii: %4d", status, data, ascii);
    }
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

    switch(c){
//A
        case 0x1e:
            if((capslock || shiftState) && (capslock != shiftState))
        return 0x41;
    return 0x61;
//B
        case 0x30:
            if((capslock || shiftState) && (capslock != shiftState))
              return 0x42;
            return 0x62;
//C
        case 0x2e:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x43;
            return 0x63;
//D
        case 0x20:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x44;
            return 0x64;
//E
        case 0x12:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x45;
            return 0x65;
//F
        case 0x21:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x46;
            return 0x66;
//G
        case 0x22:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x47;
            return 0x67;
//H
        case 0x23:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x48;
            return 0x68;

//I
        case 0x17:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x69;
            return 0x49;
//J
        case 0x24:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x4a;
        return 0x6a;
//K
        case 0x25:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x4b;
            return 0x6b;
//L
        case 0x26:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x4c;
            return 0x6c;
//M
        case 0x32:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x4d;
            return 0x6d;
//N
        case 0x31:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x4e;
            return 0x6e;
//O
        case 0x18:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x4f;
            return 0x6f;
//P
        case 0x19:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x50;
            return 0x70;
//Q
        case 0x10:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x51;
            return 0x71;
//R
        case 0x13:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x52;
            return 0x72;
//S
        case 0x1f:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x53;
            return 0x73;
//T
        case 0x14:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x54;
        return 0x74;
//U
        case 0x16:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x55;
            return 0x75;
//V
        case 0x2f:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x56;
            return 0x76;

//W
        case 0x11:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x57;
            return 0x77;
//X
        case 0x2d:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x58;
            return 0x78;
//Y
        case 0x15:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x59;
            return 0x79;
//Z
        case 0x2c:
            if((capslock || shiftState) && (capslock != shiftState))
                return 0x5a;
            return 0x7a;
//1
        case 0x02:
            if((shiftState))
                return 0x21;
        return 0x31;
//2
        case 0x03:
            if(shiftState)
                return 0x40;
        return 0x32;
//3
        case 0x04:
            if(shiftState)
                return 0x23;
        return 0x33;
//4
        case 0x05:
            if(shiftState)
                return 0x24;
        return 0x34;
//5
        case 0x06:
            if(shiftState)
                return 0x25;
            return 0x35;
//6
        case 0x07:
            if(shiftState)
                return 0x5e;
        return 0x36;
//7
        case 0x08:
            if(shiftState)
                return 0x26;
        return 0x37;
//8
        case 0x09:
            if(shiftState)
                return 0x2a;
        return 0x38;
//9
        case 0x0a:
            if(shiftState)
                return 0x28;
        return 0x39;
//0
        case 0x0b:
            if(shiftState)
                return 0x29;
        return 0x30;
//-
        case 0x0c:
            if(shiftState)
                return 0x5f;
        return 0x2d;
//=
        case 0x0d:
            if(shiftState)
                return 0x2b;
        return 0x3d;
//[
        case 0x1a:
            if(shiftState)
                return 0x7b;
        return 0x5b;
//]
        case 0x1b:
            if(shiftState)
                return 0x7d;
        return 0x5d;
//;
        case 0x27:
            if(shiftState)
                return 0x3a;
        return 0x3b;
//'
        case 0x28:
            if(shiftState)
                return 0x22;
        return 0x27;
//,
        case 0x33:
            if(shiftState)
                return 0x3c;
        return 0x2c;
//.
        case 0x35:
            if(shiftState)
                return 0x3e;
        return 0x2e;
///
        case 0x36:
            if(shiftState)
                return 0x3f;
        return 0x5c;
//space
        case 0x39:
            return 0x20;
//`
        case 0x29:
            if(shiftState)
                return 0x7e;
        return 0x60;
//tab
        case 0x0f:
            return 0x09;
//backspace
        case 0x0e:
            return 0x08;
    }
    return 0x00;
}

