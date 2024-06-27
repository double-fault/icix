#include <stdio.h>
#include <assert.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/isr.h>
#include <kernel/pic.h>
#include <kernel/utilities.h>
#include <kernel/vmm.h>
#include <kernel/multiboot.h>

extern int test_f();

uint32_t tick = 0;

void timer_callback(cpu_state_t regs)
{
   PIC_sendEOI(regs.int_no - 32);
   tick++;
   //printf("Tick: %c\n", tick + '0');
}

void init_timer(uint32_t frequency)
{
   // Firstly, register our timer callback.
   register_int_handler(32, &timer_callback);

   // The value we send to the PIT is the value to divide it's input clock
   // (1193180 Hz) by, to get our required frequency. Important to note is
   // that the divisor must be small enough to fit into 16-bits.
   uint32_t divisor = 1193180 / frequency;

   // Send the command byte.
   outb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   uint8_t l = (uint8_t)(divisor & 0xFF);
   uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   // Send the frequency divisor.
   outb(0x40, l);
   outb(0x40, h);
}

void kernel_main(multiboot_header_t *mboot_header) {
	/* jugaad */
	pre_init_idt();

	printf("Terminal init.. ");
	terminal_initialize();
	printf("OK\nGDT init.. ");
	init_gdt();
	printf("OK\nIDT init.. ");
	init_idt();
	printf("OK\nPaging init.. ");
	init_paging();
	printf("OK\nPIC remap.. ");
	PIC_remap(0x20, 0x28);
	PIC_disable();
	printf("OK\nPIT init.. ");
	init_timer(50);
	IRQ_clear_mask(0);
	printf("OK\n");
	printf("Hello, world! %d\n", test_f());
	printf("Interrupts status: %d\n", are_interrupts_enabled());
	printf("Booted from: %s\n", mboot_header->boot_loader_name); 
	printf("Modules cnt: %d\n", mboot_header->mods_count);

	assert(0);

	/*
	uint32_t *ptr = (uint32_t*)0xFFFFFFFF; 
	uint32_t tmp = *ptr;
	printf("value: %d\n", tmp);
	*/

	 for(;;) {
    		asm("hlt");
 	}
}
