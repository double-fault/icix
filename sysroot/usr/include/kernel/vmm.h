#ifndef _KERNEL_VMM_H
#define _KERNEL_VMM_H

#include <kernel/isr.h>
#include <kernel/pmm.h>

#include <stddef.h>
#include <stdint.h>

#define MEM_SIZE 0x1000000 // 16 MB, size of phys memory
#define PAGE_SIZE 0x1000

typedef struct _page {
	uint32_t present:1;
	uint32_t rw:1;
	uint32_t user:1;
	uint32_t accessed:1;
	uint32_t dirty:1;
	uint32_t unused:7; // unused + intel reserved bits
	uint32_t frame:20;
} page_t;

typedef struct _page_table {
	page_t pages[1024];
} page_table_t;

typedef struct _page_dir {
	page_table_t *tables[1024];

	uint32_t phys_tables[1024]; // physical addrs of the tables
				    
	uint32_t phys_addr; // physical address of phys_tables
} page_dir_t;

void init_paging();
void switch_page_dir(page_dir_t *dir);
page_t *get_page(uint32_t addr, uint32_t make, page_dir_t *dir);

void page_fault(cpu_state_t state);

#endif // _KERNEL_VMM_H

