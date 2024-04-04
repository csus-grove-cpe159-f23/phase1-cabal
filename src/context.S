/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Context Switch Functions
 */
#include <spede/machine/asmacros.h>
#include "interrupts.h"
#include "kernel.h"

//define kernel stack space
.comm kstack, KSTACK_SIZE, 1
.text

// Keyboard ISR Entry
ENTRY(isr_entry_keyboard)
    // Pass the irq number to the irq handler via the stack
    pushl $IRQ_KEYBOARD
    jmp kernel_enter

// Timer ISR Entry
ENTRY(isr_entry_timer)
    // Pass the irq number to the irq handler via the stack
    pushl $IRQ_TIMER
    jmp kernel_enter

//enter kernel context
kernel_enter:
    //save the registers
    pusha
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    //load the kernel
    movl %esp, %edx
    cld
    movw $(KDATA_SEG), %ax
    mov %ax, %ds
    mov %ax, %es
    leal kstack + KSTACK_SIZE, %esp
    pushl %edx
    //trigger Kernel Entry.
    call CNAME(kernel_context_enter)

//exit kernel context
ENTRY(kernel_context_exit)
    //load the stack pointer
    movl 4(%esp), %eax
    movl %eax, %esp
    popl %gs
    popl %fs
    popl %es
    popl %ds
    popa
    //adjust stack pointer to compensate for interrupt number
    add $4, %esp
    iret
