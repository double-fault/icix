#include <kernel/pmm.h>

#include <stddef.h>
#include <stdint.h>

extern uint32_t endkernel; // defined in boot.S
uint32_t free_addr = (uint32_t)&endkernel;

uint32_t kmalloc(size_t size, uint32_t *physical, int align) {
	if (align == 1 && (free_addr & 0x00000FFF)) {
	       free_addr &= 0XFFFFF000;
	       free_addr += 0x1000;
	}

	if (physical != NULL) {
		*physical = free_addr;
	}

	uint32_t ret = free_addr; free_addr += size;
	return ret;
}

