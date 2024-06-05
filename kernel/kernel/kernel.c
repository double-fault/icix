#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/utilities.h>

extern int test_f();

void kernel_main(void) {
	terminal_initialize();
	init_gdt();
	init_idt();
	PIC_remap(0x20, 0x28);
	printf("Hello, kernel World! %c\n", test_f());
	printf("Interrupts status: %c\n", are_interrupts_enabled() + '0');
	int x = 5 / 0;
	printf("%c\n", x);
}
