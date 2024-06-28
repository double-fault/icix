#include <kernel/pmm.h>
#include <kernel/vmm.h>

#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

extern uint32_t endkernel; // defined in boot.S
uint32_t free_addr = (uint32_t)&endkernel;

extern page_dir_t *kernel_dir; 

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

// use only after paging is enabled
uint32_t malloc(size_t size) {
	uint32_t ret = kmalloc(size, NULL, 1);
	
	for (uint32_t j = ret; j < (size + ret); j += PAGE_SIZE) {
		page_t *page = get_page(j, 1, kernel_dir);
		assert(page != NULL);
		if (!page->present) pfa_alloc(page, 0, 0);
	}
	return ret;
}

