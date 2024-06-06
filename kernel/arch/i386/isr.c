#include <kernel/isr.h>

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

__attribute__((aligned(0x10))) 
static idt_entry_t idt[NUM_IDT_ENTRIES]; 
static idt_ptr_t idt_ptr;

isr_t interrupt_handlers[NUM_IDT_ENTRIES];

extern void* isr_stub_table[];

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->attributes     = flags;
    descriptor->isr_high       = (uint32_t)isr >> 16;
    descriptor->reserved       = 0;
}

void pre_init_idt() {
	/* memset is causing some weird linker issue? 
	memset(interrupt_handlers, 0, sizeof interrupt_handlers);*/
	for (uint32_t i = 0; i < NUM_IDT_ENTRIES; ++i) interrupt_handlers[i] = 0;
}

void init_idt() {
    idt_ptr.base = (uintptr_t)&idt[0];
    idt_ptr.limit = (uint16_t)sizeof(idt_entry_t) * NUM_IDT_ENTRIES - 1;

    for (uint32_t vector = 0; vector < NUM_IDT_ENTRIES; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    }

    __asm__ volatile ("lidt %0" : : "m"(idt_ptr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}

void interrupt_handler(cpu_state_t state) {
	if (interrupt_handlers[state.int_no] != 0) {
		interrupt_handlers[state.int_no](state);
		return;
	}

	printf("Exception occurred with no. %c\n", state.int_no + '0');
       	__asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

void register_int_handler(uint32_t num, isr_t handler) {
	interrupt_handlers[num] = handler;
}


