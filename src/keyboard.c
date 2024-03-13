#include <spede/stdio.h>
#include <spede/machine/io.h>

#include "kernel.h"
#include "keyboard.h"
#include "tty.h"
#include "interrupts.h"

// Keyboard data port
#define KBD_PORT_DATA           0x60

// Keyboard status port
#define KBD_PORT_STAT           0x64

// Keyboard status bits (CTRL, ALT, SHIFT, CAPS, NUMLOCK)
#define KEY_STATUS_CTRL         0x01
#define KEY_STATUS_ALT          0x02
#define KEY_STATUS_SHIFT        0x04
#define KEY_STATUS_CAPS         0x08
#define KEY_STATUS_NUMLOCK      0x10

// Keyboard scancode definitions
#define KEY_CTRL_L              0x1D
#define KEY_CTRL_R              0xE01D

#define KEY_ALT_L               0x38
#define KEY_ALT_R               0xE038

#define KEY_SHIFT_L             0x2A
#define KEY_SHIFT_R             0x36

#define KEY_CAPS                0x3A
#define KEY_NUMLOCK             0x45

// Macros for handling keyboard presses or releases
#define KEY_PRESSED(c)          ((c & 0x80) == 0)
#define KEY_RELEASED(c)         ((c & 0x80) != 0)

// Macros for testing key status combinations
#define KEY_STATUS_ALL(stat, test) ((stat & test) == test)
#define KEY_STATUS_ANY(stat, test) ((stat & test) != 0)

// If this sequence of keys is pressed along with another character,
// the kernel debug command function will be called
#define KEY_KERNEL_DEBUG        (KEY_STATUS_CTRL)

// Bit-map to keep track of CTRL, ALT, SHIFT, CAPS, NUMLOCK
//
// When any of these keys are pressed, the appropriate bit
// should be set. When released, the bit should be cleared.
//   CTRL, ALT, SHIFT
//
// When any of these keys are pressed and then released, the
// appropriate bits should be toggled:
//   CAPS, NUMLOCK
static unsigned int kbd_status = 0x0;
static unsigned int esc_status = 0;

// Primary keymap
static const char keyboard_map_primary[] = {
    KEY_NULL,           /* 0x00 - Null */
    KEY_ESCAPE,         /* 0x01 - Escape  */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b',               /* 0x0e - Backspace */
    '\t',               /* 0x0f - Tab */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    '\n',               /* 0x1e - Enter */
    KEY_NULL,           /* 0x1d - Left Ctrl */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    KEY_NULL,           /* 0x2a - Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    KEY_NULL,           /* 0x36 - Right Shift */
    KEY_NULL,           /* 0x37 - Print Screen */
    KEY_NULL,           /* 0x38 - Left Alt */
    ' ',                /* 0x39 - Spacebar */
    KEY_NULL,           /* 0x3a - CapsLock */
    KEY_F1,             /* 0x3b - F1 */
    KEY_F2,             /* 0x3c - F2 */
    KEY_F3,             /* 0x3d - F3 */
    KEY_F4,             /* 0x3e - F4 */
    KEY_F5,             /* 0x3f - F5 */
    KEY_F6,             /* 0x40 - F6 */
    KEY_F7,             /* 0x41 - F7 */
    KEY_F8,             /* 0x42 - F8 */
    KEY_F9,             /* 0x43 - F9 */
    KEY_F10,            /* 0x44 - F10 */
    KEY_NULL,           /* 0x45 - NumLock */
    KEY_NULL,           /* 0x46 - ScrollLock */
    '7',                /* 0x47 - Numpad 7 */
    KEY_UP, //'8',                /* 0x48 - Numpad 8 */
    '9',                /* 0x49 - Numpad 9 */
    '-',                /* 0x4a - Numpad Minus */
    KEY_LEFT, //'4',                /* 0x4b - Numpad 4 */
    '5',                /* 0x4c - Numpad 5 */
    KEY_RIGHT, //'6',                /* 0x4d - Numpad 6 */
    '+',                /* 0x4e - Numpad Plus */
    '1',                /* 0x4f - Numpad 1 */
    KEY_DOWN, //'2',                /* 0x50 - Numpad 2 */
    '3',                /* 0x51 - Numpad 3 */
    KEY_INSERT,         /* 0x52 - Insert */
    KEY_DELETE,         /* 0x53 - Delete */
    KEY_NULL,           /* 0x54 */
    KEY_NULL,           /* 0x55 */
    KEY_NULL,           /* 0x56 */
    KEY_F11,            /* 0x57 - F11 */
    KEY_F12,            /* 0x58 - F12 */
    KEY_NULL,           /* 0x59 */
    KEY_NULL,           /* 0x5a */
    KEY_NULL,           /* 0x5b */
    KEY_NULL,           /* 0x5c */
    KEY_NULL,           /* 0x5d */
    KEY_NULL,           /* 0x5e */
    KEY_NULL,           /* 0x5f */
    KEY_NULL,           /* 0x60 */
    KEY_NULL,           /* 0x61 */
    KEY_NULL,           /* 0x62 */
    KEY_NULL,           /* 0x63 */
    KEY_NULL,           /* 0x64 */
    KEY_NULL,           /* 0x65 */
    KEY_NULL,           /* 0x66 */
    KEY_NULL,           /* 0x67 */
    KEY_NULL,           /* 0x68 */
    KEY_NULL,           /* 0x69 */
    KEY_NULL,           /* 0x6a */
    KEY_NULL,           /* 0x6b */
    KEY_NULL,           /* 0x6c */
    KEY_NULL,           /* 0x6d */
    KEY_NULL,           /* 0x6e */
    KEY_NULL,           /* 0x6f */
    KEY_NULL,           /* 0x70 */
    KEY_NULL,           /* 0x71 */
    KEY_NULL,           /* 0x72 */
    KEY_NULL,           /* 0x73 */
    KEY_NULL,           /* 0x74 */
    KEY_NULL,           /* 0x75 */
    KEY_NULL,           /* 0x76 */
    KEY_NULL,           /* 0x77 */
    KEY_NULL,           /* 0x78 */
    KEY_NULL,           /* 0x79 */
    KEY_NULL,           /* 0x7a */
    KEY_NULL,           /* 0x7b */
    KEY_NULL,           /* 0x7c */
    KEY_NULL,           /* 0x7d */
    KEY_NULL,           /* 0x7e */
    KEY_NULL            /* 0x7f */
};

// Secondary keymap (when CAPS ^ SHIFT is enabled)
static const char keyboard_map_secondary[] = {
    KEY_NULL,           /* 0x00 - Undefined */
    KEY_ESCAPE,         /* 0x01 - Escape */
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b',               /* 0x0e - Backspace */
    '\t',               /* 0x0f - Tab */
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
    '\n',               /* 0x1e - Enter */
    KEY_NULL,           /* 0x1d - Left Ctrl */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    KEY_NULL,           /* 0x2a - Left Shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    KEY_NULL,           /* 0x36 - Right Shift */
    KEY_NULL,           /* 0x37 - Print Screen */
    KEY_NULL,           /* 0x38 - Left Alt */
    ' ',
    KEY_NULL,           /* 0x3a - CapsLock */
    KEY_F1,             /* 0x3b - F1 */
    KEY_F2,             /* 0x3c - F2 */
    KEY_F3,             /* 0x3d - F3 */
    KEY_F4,             /* 0x3e - F4 */
    KEY_F5,             /* 0x3f - F5 */
    KEY_F6,             /* 0x40 - F6 */
    KEY_F7,             /* 0x41 - F7 */
    KEY_F8,             /* 0x42 - F8 */
    KEY_F9,             /* 0x43 - F9 */
    KEY_F10,            /* 0x44 - F10 */
    KEY_NULL,           /* 0x45 - NumLock */
    KEY_NULL,           /* 0x46 - ScrollLock */
    KEY_HOME,           /* 0x47 - Home */
    KEY_UP,             /* 0x48 - Up Arrow */
    KEY_PAGE_UP,        /* 0x49 - Page Up */
    '-',                /* 0x4a - Numpad minus */
    KEY_LEFT,           /* 0x4b - Left Arrow */
    KEY_NULL,           /* 0x4c - Numpad Center */
    KEY_RIGHT,          /* 0x4d - Right Arrow */
    '+',                /* 0x4e - Numpad plus */
    KEY_END,            /* 0x4f - Page End */
    KEY_DOWN,           /* 0x50 - Down Arrow */
    KEY_PAGE_DOWN,      /* 0x51 - Page Down */
    KEY_INSERT,         /* 0x52 - Insert */
    KEY_DELETE,         /* 0x53 - Delete */
    KEY_NULL,           /* 0x54 */
    KEY_NULL,           /* 0x55 */
    KEY_NULL,           /* 0x56 */
    KEY_F11,            /* 0x57 - F11 */
    KEY_F12,            /* 0x58 - F12 */
    KEY_NULL,           /* 0x59 */
    KEY_NULL,           /* 0x5a */
    KEY_NULL,           /* 0x5b */
    KEY_NULL,           /* 0x5c */
    KEY_NULL,           /* 0x5d */
    KEY_NULL,           /* 0x5e */
    KEY_NULL,           /* 0x5f */
    KEY_NULL,           /* 0x60 */
    KEY_NULL,           /* 0x61 */
    KEY_NULL,           /* 0x62 */
    KEY_NULL,           /* 0x63 */
    KEY_NULL,           /* 0x64 */
    KEY_NULL,           /* 0x65 */
    KEY_NULL,           /* 0x66 */
    KEY_NULL,           /* 0x67 */
    KEY_NULL,           /* 0x68 */
    KEY_NULL,           /* 0x69 */
    KEY_NULL,           /* 0x6a */
    KEY_NULL,           /* 0x6b */
    KEY_NULL,           /* 0x6c */
    KEY_NULL,           /* 0x6d */
    KEY_NULL,           /* 0x6e */
    KEY_NULL,           /* 0x6f */
    KEY_NULL,           /* 0x70 */
    KEY_NULL,           /* 0x71 */
    KEY_NULL,           /* 0x72 */
    KEY_NULL,           /* 0x73 */
    KEY_NULL,           /* 0x74 */
    KEY_NULL,           /* 0x75 */
    KEY_NULL,           /* 0x76 */
    KEY_NULL,           /* 0x77 */
    KEY_NULL,           /* 0x78 */
    KEY_NULL,           /* 0x79 */
    KEY_NULL,           /* 0x7a */
    KEY_NULL,           /* 0x7b */
    KEY_NULL,           /* 0x7c */
    KEY_NULL,           /* 0x7d */
    KEY_NULL,           /* 0x7e */
    KEY_NULL            /* 0x7f */
};


/*
 *
 */
void keyboard_irq_handler(void) {
    unsigned int c = keyboard_poll();

    if (c) {
        tty_update(c);
    }
}


#define KEY_PRESSED(c) ((c & 0x80) == 0)
#define KEY_RELEASED(c) ((c & 0x80) != 0)

/*static const char keyboard_map_primary[] = {
   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'
};*/

//static const char keyboard_map_secondary[]

static int shiftState;
static int capslock;
static int altState;
static int ctrlState;
/**
 * Initializes keyboard data structures and variables
 */
void keyboard_init() {

    kernel_log_info("Initializing keyboard driver");
    shiftState = 0;
    capslock = 0;
    ctrlState = 0;
    altState = 0;
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
        if(data == 0x2a || data == 0x36 || data == 0xAA || data == 0xB6){
                if(KEY_PRESSED(data)){
                    shiftState = 1;
                }else{
                    shiftState = 0;
                }
        }
        if(data == 0x38 || data == 0xb8) {
            if(KEY_PRESSED(data)){
                altState = 1;
            }else{
                altState = 0;
            }
        }
        if(data == 0x9d || data == 0x1d){
            if(KEY_PRESSED(data)){
                ctrlState = 1;
            }else{
                ctrlState = 0;
            }
        }
        if(data == 0xba){
            if(capslock == 1){
                capslock = 0;
            }else{
                capslock = 1;
            }
        }
        ascii = keyboard_decode(data);
        if(ctrlState){
            kernel_command(ascii); 
            return 0;
        }
        kernel_log_trace("Key Pressed. status: %4d Keycode: %5d ascii: %4d", status, data, ascii);
    }
    return ascii;
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
                return 0x49;
            return 0x69;
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
        case 0x34:
            if(shiftState)
                return 0x3e;
        return 0x2e;
///
        case 0x35:
            if(shiftState)
                return 0x3f;
        return 0x2f;
//space
        case 0x39:
            return 0x20;
//`
        case 0x29:
            if(shiftState)
                return 0x7e;
        return 0x60;
//\|
        case 0x2b:
            if(shiftState)
                return 0x7c;
            return 0x5c;
//tab
        case 0x0f:
            return 0x09;
//backspace
        case 0x0e:
            return 0x08;
//enter
        case 0x1c:
            return 0x0a;
//ctrl
    
default:
        return 0x00;
    }
}


