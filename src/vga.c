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

/**
 * Global variables in this file scope
 */

/**
 * Initializes the VGA driver and configuration
 *  - Defaults variables
 *  - Clears the screen
 */
void vga_init(void) {
    kernel_log_info("Initializing VGA driver");

    // Clear the screen
}

/**
 * Clears the VGA output and sets the background and foreground colors
 */
void vga_clear(void) {
    // Clear all character data, set the foreground and background colors
    // Set the cursor position to the top-left corner (0, 0)
}

/**
 * Clears the background color for all characters and sets to the
 * specified value
 *
 * @param bg background color value
 */
void vga_clear_bg(int bg) {
    // Iterate through all VGA memory and set only the background color bits
}

/**
 * Clears the foreground color for all characters and sets to the
 * specified value
 *
 * @param fg foreground color value
 */
void vga_clear_fg(int fg) {
    // Iterate through all VGA memory and set only the foreground color bits.
}

/**
 * Enables the VGA text mode cursor
 */
void vga_cursor_enable(void) {
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
}

/**
 * Indicates if the VGA text mode cursor is enabled or disabled
 */
bool vga_cursor_enabled(void) {
    return false;
}

/**
 * Sets the vga text mode cursor position to the current VGA row/column
 * position if the cursor is enabled
 */
void vga_cursor_update(void) {
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
        // Calculate the cursor position as an offset into
        // memory (unsigned short value)

        // Set the VGA Cursor Location High Register (0x0F)
        //   Should be the least significant byte (0x??<00>)

        // Set the VGA Cursor Location Low Register (0x0E)
        //   Should be the most significant byte (0x<00>??)
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
}

/**
 * Gets the current row position
 * @return integer value of the row (between 0 and VGA_HEIGHT-1)
 */
int vga_get_row(void) {
    return 0;
}

/**
 * Gets the current column position
 * @return integer value of the column (between 0 and VGA_WIDTH-1)
 */
int vga_get_col(void) {
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
}

/**
 * Gets the current background color
 * @return background color value
 */
int vga_get_bg(void) {
    return 0;
}

/**
 * Sets the foreground/text color.
 *
 * Does not modify any existing foreground colors, only sets it for
 * new operations.
 *
 * @param color - background color
 */
void vga_set_fg(int fg) {
}

/**
 * Gets the current foreground color
 * @return foreground color value
 */
int vga_get_fg(void) {
    return 0;
}

/**
 * Prints a character on the screen without modifying the cursor or other attributes
 *
 * @param c - Character to print
 */
void vga_setc(unsigned char c) {
    unsigned short *vga_buf = VGA_BASE;
    vga_buf[0] = VGA_CHAR(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY, c);
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
    unsigned short *vga_buf = VGA_BASE;
    vga_buf[0] = VGA_CHAR(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY, c);

    // Handle scecial characters

    // Handle end of lines

    // Wrap-around to the top/left corner

    // Update the text mode cursor, if enabled
}

/**
 * Prints a string on the screen at the current cursor (row/column) position
 *
 * @param s - string to print
 */
void vga_puts(char *s) {
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
}

