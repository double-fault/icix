#define AC_AC 0x1       // access
#define AC_RW 0x2       // readable for code selector & writeable for data selector
#define AC_DC 0x4       // direcion
#define AC_EX 0x8       // executable, code segment
#define AC_RE 0x10 
#define AC_PR 0x80      // persent in memory

#define AC_DPL_KERN 0x0  // RING 0 kernel level
#define AC_DPL_USER 0x60 // RING 3 user level

#define GDT_GR  0x8     // limit in 4k blocks
#define GDT_SZ  0x4     // 32 bit protect mode

#include <stdio.h>

int main(void) {
	printf("%x %x %x %x\n", AC_RW|AC_EX|AC_DPL_KERN|AC_PR, GDT_GR|GDT_SZ, AC_RW|AC_DPL_KERN|AC_PR, GDT_GR|GDT_SZ);
	return 0;
}

