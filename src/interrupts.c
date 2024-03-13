/** //f
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Interrupt handling functions
 */
//d
//f include stuff!
#include <spede/machine/io.h>
#include <spede/machine/proc_reg.h>
#include <spede/machine/seg.h>
#include <spede/string.h>

#include <stdbool.h>

#include "kernel.h"
#include "interrupts.h"
#include "bit.h"
//d
//f declare constants!
// Maximum number of ISR handlers
#define IRQ_MAX      0xf0
// PIC Definitions
#define PIC1_BASE   0x20            // base address for PIC primary controller
#define PIC2_BASE   0xa0            // base address for PIC secondary controller
#define PIC1_CMD    PIC1_BASE       // address for issuing commands to PIC1
#define PIC1_DATA   (PIC1_BASE+1)   // address for setting data for PIC1
#define PIC2_CMD    PIC2_BASE       // address for issuing commands to PIC2
#define PIC2_DATA   (PIC2_BASE+1)   // address for setting data for PIC2
#define PIC_EOI     0x20            // PIC End-of-Interrupt command
#define IRQ_PER_PIC 8               // There are 8 irqs assigned to each PIC
#define PIC_IRQ_MIN 32              // The PIC irqs start at 32
//d
// Interrupt descriptor table //f
struct i386_gate *idt = NULL;
//d
// Interrupt handler table //f
// Contains an array of function pointers associated with
// the various interrupts to be handled
void (*irq_handlers[IRQ_MAX])();
//d
void interrupts_enable(void) { //f
    /** //f
     * Enable interrupts with the CPU
     */
    //d
    kernel_log_trace("interrupts: enabling");
    asm("sti");
}
//d
void interrupts_disable(void) { //f
    /** //f
     * Disable interrupts with the CPU
     */
    //d
    kernel_log_trace("interrupts: disabling");
    asm("cli");
}
//d
void interrupts_irq_handler(int irq) { //f
    /** //f
     * Handles the specified interrupt by dispatching to the registered function
     * @param interrupt - interrupt number
     */
    //d
    if (irq < 0 || irq >= IRQ_MAX) {
        kernel_panic("interrupts: Invalid IRQ %d (0x%02x)", irq, irq);
        return;
    }

    if (irq_handlers[irq] == NULL) {
        kernel_panic("interrupts: No handler registered for IRQ %d (0x%02x)", irq, irq);
        return;
    }

    irq_handlers[irq]();

    /* If the IRQ originates from the PIC, dismiss the IRQ */
    if (irq >= 0x20 && irq <= 0x2F) {
        pic_irq_dismiss(irq - 0x20);
    }
}
//d
void interrupts_irq_register(int irq, void (*entry)(), void (*handler)()) { //f
    /* //f
     * Registers the appropriate IDT entry and handler function for the
     * specified interrupt.
     *
     * @param interrupt - interrupt number
     * @param entry - the function to run when the interrupt occurs
     * @param handler - the function to be called to process the the interrupt
     */
    //d
    kernel_log_trace("Registering an interrupt irq %d", irq);
    if (irq < 0 || irq >= IRQ_MAX) {
        kernel_panic("interrupts: Invalid IRQ %d (0x%02x)", irq, irq);
        return;
    }

    if (!entry) {
        kernel_panic("interrupts: Invalid IDT entry for IRQ %d (0x%02x)", irq, irq);
        return;
    }

    if (!handler) {
        kernel_panic("interrupts: Invalid handler for IRQ %d (0x%02x)", irq, irq);
        return;
    }

    // Add the entry to the IDT
    fill_gate(&idt[irq], (int)entry, get_cs(), ACC_INTR_GATE, 0);
    kernel_log_debug("interrupts: IRQ %d (0x%02x) IDT entry added", irq, irq);

    /* Add the ISR handler to the table */
    irq_handlers[irq] = handler;
    kernel_log_debug("interrupts: IRQ %d (0x%02x) handler added", irq, irq);

    /* If the interrupt originates from the PIC, enable IRQs */
    if (irq >= 0x20 && irq <= 0x2F) {
        pic_irq_enable(irq);
    }
    kernel_log_info("interrupts: IRQ %d (0x%02x) registered)", irq, irq);
}
//d
int get_data_address_from_irq(int irq){ //f
    if(irq<IRQ_PER_PIC+PIC_IRQ_MIN){
        return PIC1_DATA;
    }
    return PIC2_DATA;
}
//d
int get_pic_index_from_irq(int irq){ //f
    return irq%IRQ_PER_PIC;
}
//d
void pic_irq_enable(int irq) { //f
    /** //f
     * Enables the specified IRQ on the PIC
     *
     * @param irq - IRQ that should be enabled
     * @note IRQs > 0xf will be remapped
     */
    //d
    //f select the appropriate data address and index
    int pic_data_address = get_data_address_from_irq(irq);
    int pic_index = get_pic_index_from_irq(irq);
    //d
    //f Read the current mask
    unsigned int old_mask = inportb(pic_data_address);
    //d
    //f Clear the associated bit in the mask to enable the IRQ
    unsigned int modified_mask = bit_clear(old_mask, pic_index); 
    //d
    //f Write the mask out to the PIC
    outportb(pic_data_address, modified_mask);
    //d
}
//d
void pic_irq_disable(int irq) { //f
    /** //f
     * Disables the specified IRQ via the PIC
     *
     * @param irq - IRQ that should be disabled
     */
    //d
    //f select the appropriate data address and index
    int pic_data_address = get_data_address_from_irq(irq);
    int pic_index = get_pic_index_from_irq(irq);
    //d
    //f Read the current mask
    unsigned int old_mask = inportb(pic_data_address);
    //d
    //f Set the associated bit in the mask to enable the IRQ
    unsigned int modified_mask = bit_set(old_mask, pic_index); 
    //d
    //f Write the mask out to the PIC
    outportb(pic_data_address, modified_mask);
    //d
}
//d
int pic_irq_enabled(int irq) { //f
    /** //f
     * Queries if the given IRQ is enabled on the PIC
     *
     * @param irq - IRQ to check
     * @return - 1 if enabled, 0 if disabled
     */
    //d
    //f select the appropriate data address and index
    int pic_data_address = get_data_address_from_irq(irq);
    int pic_index = get_pic_index_from_irq(irq);
    //d
    //f retrieve the mask
    unsigned int pic_mask = inportb(pic_data_address);
    //d
    //f determine whether the specific index is enabled!
    bool is_enabled = !(bool)bit_test(pic_mask, pic_index);
    //d
    return is_enabled;
}
//d
void pic_irq_dismiss(int irq) { //f
    /** //f
     * Dismisses an interrupt by sending the EOI command to the appropriate
     * PIC device(s). If the IRQ is assosciated with the secondary PIC, the
     * EOI command must be issued to both since the PICs are dasiy-chained.
     *
     * @param irq - IRQ to be dismissed
     */
    //d
    //f Send EOI to the secondary PIC, if needed
    if (irq >= IRQ_PER_PIC){
        outportb(PIC2_CMD, PIC_EOI);
    }
    //d
    //f Send EOI to the primary PIC, ALWAYS
        outportb(PIC1_CMD, PIC_EOI);
    //d
}
//d
void interrupts_init() { //f
    /** //f
     * Interrupt initialization
     */
    //d
    kernel_log_info("Initializing Interrupts");
    // Obtain the IDT base address
    idt = get_idt_base();
    memset(irq_handlers, 0, sizeof(irq_handlers));
}
//d

