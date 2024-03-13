#include <spede/stdarg.h>
#include <spede/stdio.h>

#include "bit.h"
#include "io.h"
#include "kernel.h"
#include "vga.h"

/**
 * Forward Declarations
 */

void vga_cursor_update(void);
void vga_clear_char(void);
void vga_cursor_toggle(void);

/**
 * Global variables in this file scope
 */
static bool cursor_enabled = true;
static int fg_color = VGA_COLOR_WHITE;
static int bg_color = VGA_COLOR_RED;

/**
 * Initializes the VGA driver and configuration
 *  - Defaults variables
 *  - Clears the screen
 */
void vga_init(void) {
    kernel_log_info("Initializing VGA driver");
    // Clear the screen
    vga_clear();
    kernel_log_info("Initializing Done");

}

/**
 * Clears the VGA output and sets the background and foreground colors
 */
void vga_clear(void) {
    // Clear all character data, set the foreground and background colors
    kernel_log_info("vga clear called");
    vga_clear_bg(vga_get_bg());
    vga_clear_fg(vga_get_fg());
    vga_clear_char();
    // Set the cursor position to the top-left corner (0, 0)
    vga_set_rowcol(0,0);
}

/**
 * Clears the background color for all characters and sets to the
 * specified value
 *
 * @param bg background color value
 */
void vga_clear_bg(int bg) {
    kernel_log_trace("bg clear");
    // Iterate through all VGA memory and set only the background color bits
    unsigned short *vga_buf = VGA_BASE;
    unsigned short clear_value = VGA_ATTR(bg, 0);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga_buf[i] = (vga_buf[i] & 0x0FFF ) | (clear_value << 8);
    }

}

/**
 * Clears the foreground color for all characters and sets to the
 * specified value
 *
 * @param fg foreground color value
 */
void vga_clear_fg(int fg) {
    kernel_log_trace("fg clear");
    // Iterate through all VGA memory and set only the foreground color bits.
    unsigned short *vga_buf = VGA_BASE;
    unsigned short clear_value = VGA_ATTR(0, fg);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga_buf[i] = (vga_buf[i] & 0xF0FF) | clear_value<<8;
    }
}

void vga_clear_char(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        VGA_BASE[i] = (VGA_BASE[i] & 0xFF00 );
    }
}

/**
 * Enables the VGA text mode cursor
 */
void vga_cursor_enable(void) {
    cursor_enabled = true;
outportb(VGA_PORT_ADDR, 0x0A); 
outportb(VGA_PORT_DATA, (inportb(VGA_PORT_DATA) & 0xDF));
    vga_cursor_update();

    // All operations will consist of writing to the address port which
    // register should be set, followed by writing to the data port the value
    // to set for the specified register

    // The cursor will be drawn between the scanlines defined
    // in the following registers:
    //   0x0A Cursor Start Register
    //   0x0B Cursor End Register

    // Bit 5 in the cursor start register (0x0A) will enable or disable the cursor:
    //   0 - Cursor enabled
    //   1 - Cursor disabled

    // The cursor will be displayed as a series of horizontal lines that create a
    // "block" or rectangular symbol. The position/size is determined by the "starting"
    // scanline and the "ending" scanline. Scanlines range from 0x0 to 0xF.

    // In both the cursor start register and cursor end register, the scanline values
    // are specified in bits 0-4

    // To ensure that we do not change bits we are not intending,
    // read the current register state and mask off the bits we
    // want to save

    // Set the cursor starting scanline (register 0x0A, cursor start register)

    // Set the cursor ending scanline (register 0x0B, cursor end register)
    // Ensure that bit 5 is not set so the cursor will be enabled

    // Since we may need to update the vga text mode cursor position in
    // the future, ensure that we track (via software) if the cursor is
    // enabled or disabled

    // Update the cursor location once it is enabled
}

/**
 * Disables the VGA text mode cursor
 */
void vga_cursor_disable(void) {
    cursor_enabled = false;
    // All operations will consist of writing to the address port which
    // register should be set, followed by writing to the data port the value
    // to set for the specified register

    // The cursor will be drawn between the scanlines defined
    // in the following registers:
    //   0x0A Cursor Start Register
    //   0x0B Cursor End Register

    // Bit 5 in the cursor start register (0x0A) will enable or disable the cursor:
    //   0 - Cursor enabled
    //   1 - Cursor disabled

    // Since we are disabling the cursor, we can simply set the bit of interest
    // as we won't care what the current cursor scanline start/stop values are

    // Since we may need to update the vga text mode cursor position in
    // the future, ensure that we track (via software) if the cursor is
    // enabled or disabled
    outportb(VGA_PORT_ADDR, 0x0A);
    outportb(VGA_PORT_DATA, (inportb(VGA_PORT_DATA) | 0X20));


}

void vga_cursor_toggle(void){ //toggles the cursor on or off when called
    if(vga_cursor_enabled()){
        vga_cursor_disable();
    }else{
        vga_cursor_enable();
    }
}

/**
 * Indicates if the VGA text mode cursor is enabled or disabled
 */
bool vga_cursor_enabled(void) {
    return cursor_enabled;
}

/**
 * Sets the vga text mode cursor position to the current VGA row/column
 * position if the cursor is enabled
 */
void vga_cursor_update(void) {//UNFINISHED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // All operations will consist of writing to the address port which
    // register should be set, followed by writing to the data port the value
    // to set for the specified register

    // The cursor position is represented as an unsigned short (2-bytes). As
    // VGA register values are single-byte, the position representation is
    // split between two registers:
    //   0x0F Cursor Location High Register
    //   0x0E Cursor Location Low Register

    // The Cursor Location High Register is the least significant byte
    // The Cursor Location Low Register is the most significant byte

    // If the cursor is enabled:
    if (vga_cursor_enabled()) {
        // Calculate the cursor position as an offset into
        // memory (unsigned short value)
        int position = vga_get_row() * VGA_WIDTH + vga_get_col();

        // Set the VGA Cursor Location High Register (0x0F)
        //   Should be the least significant byte (0x??<00>)

        // Step 1: Select the register by writing its address to the VGA address port
        outportb(VGA_PORT_ADDR,0x0F);

        // Step 2: Write the data to be stored in the register to the VGA data port
        outportb(VGA_PORT_DATA, (position & 0xFF));



        // Set the VGA Cursor Location Low Register (0x0E)
        //   Should be the most significant byte (0x<00>??)
        outportb(VGA_PORT_ADDR, 0x0E);
        outportb(VGA_PORT_DATA, ((position >> 8) & 0xFF));
    }

}

/**
 * Sets the current row/column position
 *
 * @param row position (0 to VGA_HEIGHT-1)
 * @param col position (0 to VGA_WIDTH-1)
 * @notes If the input parameters exceed the valid range, the position
 *        will be set to the range boundary (min or max)
 */
void vga_set_rowcol(int row, int col) {
    // Update the text mode cursor (if enabled)
    if (row < 0)
        row = 0;
    else if (row >= VGA_HEIGHT)
        row = VGA_HEIGHT - 1;

    if (col < 0)
        col = 0;
    else if (col >= VGA_WIDTH)
        col = VGA_WIDTH - 1;


    unsigned short position = (row * VGA_WIDTH) + col;
    outportb(VGA_PORT_ADDR, 0x0F);
    outportb(VGA_PORT_DATA, (position & 0xFF));
    outportb(VGA_PORT_ADDR, 0x0E);
    outportb(VGA_PORT_DATA, ((position >> 8) & 0xFF));

}

/**
 * Gets the current row position
 * @return integer value of the row (between 0 and VGA_HEIGHT-1)
 */
int vga_get_row(void) {
    unsigned short position = 0;
    outportb(VGA_PORT_ADDR, 0x0F);
    position |= inportb(VGA_PORT_DATA);
    outportb(VGA_PORT_ADDR, 0x0E);
    position |= (inportb(VGA_PORT_DATA) << 8);
    return position / VGA_WIDTH;}

/**
 * Gets the current column position
 * @return integer value of the column (between 0 and VGA_WIDTH-1)
 */
int vga_get_col(void) {
    unsigned short position = 0;
    outportb(VGA_PORT_ADDR, 0x0F);
    position |= inportb(VGA_PORT_DATA);
    outportb(VGA_PORT_ADDR, 0x0E);
    position |= (inportb(VGA_PORT_DATA) << 8);
    return position % VGA_WIDTH;
    return 0;
}

/**
 * Sets the background color.
 *
 * Does not modify any existing background colors, only sets it for
 * new operations.
 *
 * @param bg - background color
 */
void vga_set_bg(int bg) {
    kernel_log_info("background color changed to %d",bg);
    bg_color = bg;
}

/**
 * Gets the current background color
 * @return background color value
 */
int vga_get_bg(void) {
    return bg_color;
}

/**
 * Sets the foreground/text color.
 *
 * Does not modify any existing foreground colors, only sets it for
 * new operations.
 *
 * @param color - foreground color
 */
void vga_set_fg(int fg) {
    kernel_log_info("forground color changed to %d",fg);
    fg_color = fg;
}

/**
 * Gets the current foreground color
 * @return foreground color value
 */
int vga_get_fg(void) {
    return fg_color;
}

/**
 * Prints a character on the screen without modifying the cursor or other attributes
 *
 * @param c - Character to print
 */
void vga_setc(unsigned char c) {
    //kernel_log_info("Initializing setc");
    VGA_BASE[(vga_get_row()*VGA_WIDTH)+vga_get_col()] = VGA_CHAR(vga_get_bg(), vga_get_fg(), c);//TODO
    //kernel_log_info("setc Done");

}

/**
 * Prints a character on the screen at the current cursor (row/column) position
 *
 * When a character is printed, will do the following:
 *  - Update the row and column positions
 *  - If needed, will wrap from the end of the current line to the
 *    start of the next line
 *  - If the last line is reached, the cursor position will reset to the top-left (0, 0) position
 *  - Special characters are handled as such:
 *    - tab character (\t) prints 'tab_stop' spaces
 *    - backspace (\b) character moves the character back one position,
 *      prints a space, and then moves back one position again
 *    - new-line (\n) should move the cursor to the beginning of the next row
 *    - carriage return (\r) should move the cursor to the beginning of the current row
 *
 * @param c - character to print
 */
void vga_putc(unsigned char c) {
    //unsigned short *vga_buf = VGA_BASE;
    //vga_buf[0] = VGA_CHAR(VGA_COLOR_BLACK, VGA_COLOR_WHITE, c);
    int row = vga_get_row();
    int col = vga_get_col();
    // Handle scecial characters
    if (c == '\n') {//newline
        vga_set_rowcol(row + 1, 0);
    } else if (c == '\r') {//carriage return
        vga_set_rowcol(row, 0);
    } else if (c == '\b') {//backspace
        if (col > 0) {
            vga_set_rowcol(row, col - 1);
            vga_setc(' ');
            vga_set_rowcol(row, col - 1);
        }
        // Wrap-around to the top/left corner  //MAY NEED tO BE TWEAKED!!!!!!!!!!!!!
        else
        {
            vga_set_rowcol(row-1, VGA_WIDTH-1);
            vga_setc(' ');
            vga_set_rowcol(row-1, VGA_WIDTH-1);
        }
    } else if (c == '\t') {//tab
        int tab_stop = 4;
        for (int i = 0; i < tab_stop; ++i) {
            vga_putc(' ');
        }
    } else {
        vga_setc(c);
        if(col == VGA_WIDTH-1){
            vga_set_rowcol(row + 1, 0);
        }else{
            vga_set_rowcol(row, col + 1);
        }
    }

}

/**
 * Prints a string on the screen at the current cursor (row/column) position
 *
 * @param s - string to print
 */
void vga_puts(char *s) {
    while (*s != '\0') {
        vga_putc(*s);
        s++;
    }
}

/**
 * Prints a character on the screen at the specified row/column position and
 * with the specified background/foreground colors
 *
 * Does not modify the current row or column position
 * Does not modify the current background or foreground colors
 *
 * @param row the row position (0 to VGA_HEIGHT-1)
 * @param col the column position (0 to VGA_WIDTH-1)
 * @param bg background color
 * @param fg foreground color
 * @param c character to print
 */
void vga_putc_at(int row, int col, int bg, int fg, unsigned char c) {
    unsigned short *vga_buf = VGA_BASE;
    if (row >= 0 && row < VGA_HEIGHT && col >= 0 && col < VGA_WIDTH) {
        vga_buf[row * VGA_WIDTH + col] = VGA_CHAR(bg, fg, c);
    }
    else{
        row=VGA_HEIGHT-1;
        col=VGA_WIDTH-1;
        vga_buf[row * VGA_WIDTH + col] = VGA_CHAR(bg, fg, c);


    }
}

/**
 * Prints a string on the screen at the specified row/column position and
 * with the specified background/foreground colors
 *
 * Does not modify the current row or column position
 * Does not modify the current background or foreground colors
 *
 * @param row the row position (0 to VGA_HEIGHT-1)
 * @param col the column position (0 to VGA_WIDTH-1)
 * @param bg background color
 * @param fg foreground color
 * @param s string to print
 */
void vga_puts_at(int row, int col, int bg, int fg, char *s) {
    int i = 0;
    while (s[i] != '\0') {
        vga_putc_at(row, col + i, bg, fg, s[i]);
        i++;
    }
}

