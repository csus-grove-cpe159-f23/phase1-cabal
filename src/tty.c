#include <spede/string.h>
#include "kernel.h"
#include "timer.h"
#include "tty.h"
#include "vga.h"

// TTY Table
struct tty_t tty_table[TTY_MAX];

// Current Active TTY
struct tty_t *active_tty;

void tty_refresh(void);


/**
 * Initializes all TTY data structures and memory
 * Selects TTY 0 to be the default
 */
void tty_init(void) {
    kernel_log_info("tty: Initializing TTY driver");

    // Initialize the tty_table
    for (int i = 0; i < TTY_MAX; ++i) {
        tty_table[i].id = i;
        memset(tty_table[i].buf, 0, TTY_BUF_SIZE);
        tty_table[i].refresh = 0;
        tty_table[i].color_bg = VGA_COLOR_BLACK;
        tty_table[i].color_fg = VGA_COLOR_LIGHT_GREY;
        tty_table[i].pos_x = 0;
        tty_table[i].pos_y = 0;
        tty_table[i].pos_scroll = 0;
        //I think this should be all the intialization needed for phase 4 tty. I ak a bit iffy on my use of pointer directions here, but I think it should be ok. I should probably still ask Dylan though! -Hannah
        tty_table[i].echo=0;
        ringbuf_init(&(tty_table[i].io_output));
        ringbuf_init(&(tty_table[i].io_input));
    }

    // Select tty 0 to start with
    active_tty = &tty_table[0];

    // Register a timer callback to update the screen on a regular interval
    timer_callback_register(tty_refresh, 50, -1); // Update every 100 ms?
}

/**
 * Sets the active TTY to the selected TTY number
 * @param tty - TTY number
 */
void tty_select(int n) {
    // Set the active tty to point to the entry in the tty table
    // if a new tty is selected, the tty should trigger a refresh
    if (n >= 0 && n < TTY_MAX) {
        active_tty = &tty_table[n];
        active_tty->refresh = 1; // Trigger refresh
    } else {
        kernel_log_error("tty_select: Invalid TTY number %d", n);
    }
    kernel_log_info("tty %d selected", n);
}

/**
 * Updates the TTY with the given character
 */
void tty_update(char c) {
    if (!active_tty) {
        return;
    }

    switch (c) {
        case '\b':
            if (active_tty->pos_x != 0) {
                active_tty->pos_x--;
            } else if (active_tty->pos_y != 0) {
                active_tty->pos_y--;
                active_tty->pos_x = VGA_WIDTH - 1;
            }
            active_tty->buf[active_tty->pos_x + active_tty->pos_y * VGA_WIDTH] = ' ';
            break;
        case '\t':
            active_tty->pos_x += 4 - active_tty->pos_x % 4;
            break;
        case '\r':
            active_tty->pos_x = 0;
            break;
        case '\n':
            active_tty->pos_x = 0;
            active_tty->pos_y++;
            break;
        default:
            active_tty->buf[active_tty->pos_x + active_tty->pos_y * VGA_WIDTH] = c;
            active_tty->pos_x++;
            break;
    }

    if (active_tty->pos_x >= VGA_WIDTH) {
        active_tty->pos_x = 0;
        active_tty->pos_y++;
    }

    if (active_tty->pos_y >= VGA_HEIGHT) {
        // Scroll the screen up (copy each row to the previous)
        for (int y = 0; y < VGA_HEIGHT - 1; ++y) {
            for (int x = 0; x < VGA_WIDTH; ++x) {
                active_tty->buf[x + y * VGA_WIDTH] = active_tty->buf[x + (y + 1) * VGA_WIDTH];
            }
        }
        // Clear the last line
        for (int x = 0; x < VGA_WIDTH; ++x) {
            active_tty->buf[x + (VGA_HEIGHT - 1) * VGA_WIDTH] = ' ';
        }
        active_tty->pos_y = VGA_HEIGHT - 1;
    }

    active_tty->refresh = 1; // Set refresh flag
}





/**
 * Refreshes the tty if needed
 */
void tty_refresh(void) {
    if (!active_tty) {
        kernel_panic("No TTY is selected!");
        return;
    }
    while(ringbuf_is_empty(&(active_tty->io_output))){
        char c;
        int success = ringbuf_read(&(active_tty->io_output), &c);
        if (success==-1){
            kernel_log_error("tty io_output buffer read failed.");
        }
        tty_update(c);
    }

    if (active_tty->refresh) {
        for (int y = 0; y < VGA_HEIGHT; ++y) {
            for (int x = 0; x < VGA_WIDTH; ++x) {
                char c = active_tty->buf[x + y * VGA_WIDTH];
                vga_putc_at(x, y, active_tty->color_bg, active_tty->color_fg, c);
            }
        }
        active_tty->refresh = 0;
    }
    //kernel_log_trace("tty refresh called");
}

tty_t * tty_get(int tty){ //f
/**
 * Returns the tty structure for the given tty number
 * @param tty_number - tty number/identifier
 * @return NULL on error or pointer to entry in the tty table
 */
    if(
            (tty<0)
            ||
            (tty>=(int)(sizeof(tty_table)/sizeof(tty_t)))
    ){
            kernel_log_warn("Invalid tty number requested tty_get");
            return NULL;
    }
    return(&(tty_table[tty]));
}
//d

void tty_input(char c){
    /** //f
     * Write a character into the TTY process input buffer
     * If the echo flag is set, will also write the character into the TTY
     * process output buffer
     * @param c - character to write into the input buffer
     */
    //d
    ringbuf_write(&(active_tty->io_input), c);
    if(active_tty->echo){
        ringbuf_write(&(active_tty->io_output), c);
    }
}

//TODO as far as I am aware the scrollback buffer is not impemented. It was mentioned in the template code but not in the docs. I am marking this as OPTIONAL for that reason. -Hannah
void tty_scroll_up(void){ //f
/** //f
 * Scrolls the TTY up one line into the scrollback buffer
 * If the buffer is at the top, it will not scroll up further
 */
//d
//TODO
}
//d
void tty_scroll_down(void){ //f
/** //f
 * Scrolls the TTY down one line into the scrollback buffer
 * If the buffer is at the end, it will not scroll down further
 */
//d
//TODO
}
//d
void tty_scroll_top(void){ //f
/** //f
 * Scrolls to the top of the buffer
 */
//d
//TODO
}
//d
void tty_scroll_bottom(void){ //f
/** //f
 * Scrolls to the bottom of the buffer
 */
//d
//TODO
}
//d

int tty_get_active(){
    if(!active_tty){
        return -1;
    }
    return active_tty->id;
}

void tty_putc(struct tty_t *tty, char c) {
    // Pointer to the TTY's buffer
    char *buf = tty->buf;

    // Handle special control characters
    switch (c) {
        case '\t':
            // Handle tab character
            // Insert spaces until the next tab stop
            for (int i = 0; i < 4; i++) {
                buf[tty->pos_x++] = ' ';
                if (tty->pos_x >= TTY_WIDTH) {
                    // Handle wrapping if reaching end of line
                    tty->pos_x = 0;
                    tty->pos_y++;
                }
            }
            break;
        case '\b':
            // Handle backspace character
            if (tty->pos_x > 0) {
                // Move cursor back one position
                tty->pos_x--;
                // Overwrite character with space
                buf[tty->pos_y * TTY_WIDTH + tty->pos_x] = ' ';
            }
            break;
        case '\r':
            // Handle carriage return
            // Move cursor to beginning of line
            tty->pos_x = 0;
            break;
        case '\n':
            // Handle newline character
            // Move cursor to beginning of next line
            tty->pos_x = 0;
            tty->pos_y++;
            break;
        default:
            // Handle regular printable character
            buf[tty->pos_y * TTY_WIDTH + tty->pos_x++] = c;
            // Check if we need to wrap to next line
            if (tty->pos_x >= TTY_WIDTH) {
                tty->pos_x = 0;
                tty->pos_y++;
            }
            break;
    }

    // Handle scrolling if necessary
    if (tty->pos_y >= TTY_HEIGHT) {
        // Scroll up one line
        // Move each line up one position
        memcpy(buf, buf + TTY_WIDTH, (TTY_HEIGHT - 1) * TTY_WIDTH);
        // Clear last line
        memset(buf + (TTY_HEIGHT - 1) * TTY_WIDTH, 0, TTY_WIDTH);
        // Adjust y position
        tty->pos_y = TTY_HEIGHT - 1;
    }
}
