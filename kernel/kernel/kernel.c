#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/isr.h>
#include <kernel/pic.h>
#include <kernel/utilities.h>
#include <kernel/initrd.h>
#include <kernel/vmm.h>
#include <kernel/pmm.h>
#include <kernel/filesystem.h>
#include <kernel/disk_driver.h>
#include <kernel/multiboot.h>

extern int test_f();
extern uint32_t free_addr;

uint32_t tick = 0;

void timer_callback(cpu_state_t regs)
{
   	PIC_sendEOI(regs.int_no - 32);
   	tick++;
   	//printf("Tick: %c\n", tick + '0');
   	if (tick % 250 == 0) {
		// Triggering a page fault to show that PIT is working
		uint32_t *ptr = (uint32_t*)0xFFFFFFFF; 
		uint32_t tmp = *ptr;
		printf("value: %d\n", tmp);
   	}
}

void init_timer(uint32_t frequency)
{
	register_int_handler(32, &timer_callback);
        uint32_t divisor = 1193180 / frequency;
   	outb(0x43, 0x36);

   	uint8_t l = (uint8_t)(divisor & 0xFF);
   	uint8_t h = (uint8_t)((divisor>>8) & 0xFF);
   	outb(0x40, l);
   	outb(0x40, h);
}

void print_file(ufs *fs, int inode_num) {
	printf("\n");
	printf("contents of file (inode no. %d): \n    ", inode_num);
	char *c = (char*)malloc(sizeof(char));
	int offset = 0;
	while (ufs_read(fs, inode_num, c, offset++, 1) == 0) printf("%c", *c); 
	printf("\n");
}

void print_fs_contents(ufs *fs, int inode_num) {
	printf("\n");
	printf("contents of directory (inode no. %d) [fmt: (inode no., name)]: \n    ", inode_num);
	dir_ent_t et; int offset = 0;
	while (ufs_read(fs, inode_num, &et, offset, sizeof(dir_ent_t)) == 0) {
		offset += sizeof(dir_ent_t);
		if (et.inum == -1) break;
		printf("(%d, %s),  ", et.inum, et.name);
	}
	printf("\n");

	offset = 0;
	while (ufs_read(fs, inode_num, &et, offset, sizeof(dir_ent_t)) == 0) {
		offset += sizeof(dir_ent_t);
		if (et.inum == -1) break;

		// forgot to implement the stat function in the filesystem, so 
		// just using a random hack as all we want to do is print
		// the data to make sure everything is working as intended
		// TODO: fix this (by implementing ufs_stat)
		if (et.name[0] == 'd') {
			print_fs_contents(fs, et.inum);
			continue;
		}
		if (strcmp(et.name, ".") && strcmp(et.name, "..")) print_file(fs, et.inum);
	}
}

void kernel_main(multiboot_header_t *mboot_header) {
	/* jugaad */
	pre_init_idt();

	terminal_initialize();
	printf("Multiboot info: \n    Modules cnt: %d\n    Booted from: %s\n", mboot_header->mods_count,
			mboot_header->boot_loader_name);

	printf("IDT, GDT init..");
	init_gdt();
	init_idt();
	printf(" OK\n");
	assert(mboot_header->mods_count > 0);
	free_addr = *((uint32_t*)(mboot_header->mods_addr + 4));
	printf("Paging init.. ");
	init_paging();
	printf("OK\nPIC remap.. ");
	PIC_remap(0x20, 0x28);
	PIC_disable();
	init_timer(50);
	IRQ_clear_mask(0);
	printf("OK\nInitrd init.. ");
	disk_driver_t *initrd_driver = get_initrd_driver(mboot_header);
	printf("OK\nFilesystem init (disk = initrd).. ");
	ufs* fs = ufs_init(initrd_driver);
	printf("OK\n");
	print_fs_contents(fs, 0);

	printf("\nAfter 5 secs, PIT ISR will cause a page fault (to show that PIT, paging works):\n");

	for(;;) {
    		asm("hlt");
 	}
}
