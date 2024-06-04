#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>

extern int test_f();

void kernel_main(void) {
	terminal_initialize();
	init_gdt();
	printf("Hello, kernel World! %c\n", test_f());
}
