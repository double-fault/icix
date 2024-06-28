#include <kernel/vmm.h>
#include <kernel/isr.h>

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

extern uint32_t free_addr;

extern void load_page_dir(uint32_t* addr);
extern void enable_paging(void);

page_dir_t *kernel_dir = NULL;
page_dir_t *cur_dir = NULL;

uint32_t *frames;
uint32_t nframes;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void set_frame(uint32_t frame_addr)
{
	uint32_t frame = frame_addr/0x1000;
   	uint32_t idx = INDEX_FROM_BIT(frame);
   	uint32_t off = OFFSET_FROM_BIT(frame);
   	frames[idx] |= (0x1 << off);
}

static void clear_frame(uint32_t frame_addr)
{
   	uint32_t frame = frame_addr/0x1000;
   	uint32_t idx = INDEX_FROM_BIT(frame);
   	uint32_t off = OFFSET_FROM_BIT(frame);
   	frames[idx] &= ~(0x1 << off);
}

static uint32_t test_frame(uint32_t frame_addr)
{
   	uint32_t frame = frame_addr/0x1000;
   	uint32_t idx = INDEX_FROM_BIT(frame);
   	uint32_t off = OFFSET_FROM_BIT(frame);
   	return (frames[idx] & (0x1 << off));
}

static uint32_t first_frame()
{
   	uint32_t i, j;
   	for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
   	{
       		if (frames[i] != 0xFFFFFFFF) 
       		{
           		for (j = 0; j < 32; j++)
           		{
               			uint32_t toTest = 0x1 << j;
               			if ( !(frames[i]&toTest) )
               			{
                   			return i*4*8+j;
               			}
           		}	
       		}
   	}
	return (uint32_t)-1;
}

void pfa_alloc(page_t *page, uint32_t is_kernel, uint32_t is_writeable) {
       if (page->present == 1) return;

	uint32_t idx = first_frame();
	if (idx == (uint32_t)-1) abort();

	set_frame(idx * 0x1000);
	page->present = 1;
	page->rw = is_writeable;
	page->user = (is_kernel)?0:1;
	page->frame = idx;
}

void pfa_free(page_t *page) {
	uint32_t frame = page->frame;
	if (page->present) {
		clear_frame(frame);
		page->present = 0;
	}
}

void init_paging() {
	nframes = MEM_SIZE / PAGE_SIZE;
	frames = (uint32_t*) kmalloc(INDEX_FROM_BIT(nframes), NULL, 0);
	memset(frames, 0, INDEX_FROM_BIT(nframes));

	kernel_dir = (page_dir_t*)kmalloc(sizeof(page_dir_t), NULL, 1);
	memset(kernel_dir, 0, sizeof(page_dir_t));

	cur_dir = kernel_dir;

	// identity mapping kernel region
	for (uint32_t i = 0; i < free_addr; i += 0x1000) {
		page_t *page = get_page(i, 1, kernel_dir);
		pfa_alloc(page, 0, 0);
	}

	register_int_handler(14, &page_fault);
	
	switch_page_dir(kernel_dir);
	enable_paging();
//	printf("Paging enabled!");
//	printf("Page: %d\n", get_page(0xA0000000, 0, kernel_dir));
}

void switch_page_dir(page_dir_t *dir) {
	load_page_dir(dir->phys_tables);
}

page_t *get_page(uint32_t addr, uint32_t make, page_dir_t *dir) {
	addr /= 0x1000;
	uint32_t table_idx = addr / 1024;
	
	if (dir->tables[table_idx]) {
		return &dir->tables[table_idx]->pages[addr % 1024];
	} else if (make) {
		uint32_t phys;
		dir->tables[table_idx] = (page_table_t*)kmalloc(sizeof(page_table_t), &phys, 1); 
		memset(dir->tables[table_idx], 0, PAGE_SIZE);
		dir->phys_tables[table_idx] = phys | 0x7;
		return &dir->tables[table_idx]->pages[addr % 1024];
	}
	return NULL;
}

void page_fault(cpu_state_t state) {
	printf("Page fault!\n");
	abort();
}

