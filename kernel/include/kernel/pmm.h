#ifndef _KERNEL_MMM_H
#define _KERNEL_PMM_H

#include <stddef.h>
#include <stdint.h>

uint32_t kmalloc(size_t size, uint32_t *physical, int align);

#endif // _KERNEL_PMM_H

