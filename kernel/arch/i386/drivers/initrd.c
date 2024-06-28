#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <kernel/disk_driver.h>
#include <kernel/initrd.h>
#include <kernel/pmm.h>
#include <kernel/multiboot.h>

initrd_info_t initrd_instance;

disk_driver_t* get_initrd_driver(multiboot_header_t *mboot_header) {
	disk_driver_t* ret = (disk_driver_t*)malloc(sizeof(disk_driver_t));
	ret->init = &initrd_init;
	ret->read = &initrd_read;
	ret->write = &initrd_write;
	ret->lseek = &initrd_lseek;
	ret->clean = &initrd_clean;
	ret->fsync = &initrd_fsync;
        initrd_instance.start_addr = *((uint32_t*)mboot_header->mods_addr);
        initrd_instance.end_addr = *((uint32_t*)(mboot_header->mods_addr + 4));

	return ret;
}

uint32_t initrd_init(void) {
       initrd_instance.cursor = 0;
       return 0;
}

size_t initrd_read(void *buf, size_t size) {
	if (((uint32_t)initrd_instance.start_addr + initrd_instance.cursor + size) >= initrd_instance.end_addr) return 0;
	memcpy(buf, (uint32_t*)(initrd_instance.start_addr + initrd_instance.cursor), size);
	initrd_instance.cursor += size;
	return size;
}

size_t initrd_write([[maybe_unused]]void *buf, [[maybe_unused]]size_t size) {
	printf("Cannot write to initrd!\n");
	abort();
}

// SEEK_SET behavior only lol
uint32_t initrd_lseek(uint32_t offset) {
	if (initrd_instance.start_addr + offset > initrd_instance.end_addr) return -1;
	initrd_instance.cursor = offset;
	return 0;
}

uint32_t initrd_clean(void) {
	return 0;
}

uint32_t initrd_fsync(void) {
	return 0;
}

