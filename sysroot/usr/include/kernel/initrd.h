#ifndef _KERNEL_INITRD_H
#define _KERNEL_INITRD_H

#include <stddef.h>
#include <stdint.h>

#include <kernel/disk_driver.h>
#include <kernel/multiboot.h>

typedef struct _initrd_info_t {
	uint32_t start_addr;
	uint32_t cursor;
	uint32_t end_addr;
} initrd_info_t;

disk_driver_t* get_initrd_driver(multiboot_header_t *mboot_header);
uint32_t initrd_init(void);
size_t initrd_read(void *buf, size_t size);
size_t initrd_write(void *buf, size_t size);
uint32_t initrd_lseek(uint32_t offset);
uint32_t initrd_clean(void);
uint32_t initrd_fsync(void);

#endif // _KERNEL_INITRD_H
