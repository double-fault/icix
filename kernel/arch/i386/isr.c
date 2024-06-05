#include <kernel/tty.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

__attribute__((noreturn))
void exception_handler(void);
void exception_handler() {
	printf("Exception!!\n");
       	__asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

