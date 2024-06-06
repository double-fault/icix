#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stddef.h>
#include <stdint.h>

#define NUM_IDT_ENTRIES 256

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t     reserved;     // Set to zero
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed)) idt_ptr_t;

void pre_init_idt(void);
void init_idt(void);
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

typedef struct cpu_state {
	uint32_t cr2;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    	uint32_t int_no, err_code;
   	uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed)) cpu_state_t;

__attribute__((noreturn))
void exception_handler(void);

typedef void (*isr_t)(cpu_state_t);
void register_int_handler(uint32_t num, isr_t handler);

#endif // _KERNEL_IDT_H

