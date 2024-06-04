#include <kernel/gdt.h>

#include <stddef.h>
#include <stdint.h>

extern void _gdt_flush(uint32_t); 

void _gdt_set_gate(int32_t, uint32_t, uint32_t, uint8_t, uint8_t);

gdt_entry_t gdt_entries[NUM_GDT_ENTRIES];
gdt_ptr_t gdt_ptr;

void init_gdt() {
	gdt_ptr.limit = sizeof(gdt_entries) - 1;
	gdt_ptr.base = (uint32_t)&gdt_entries;

	_gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
        _gdt_set_gate(1, 0, 0xFFFF, 0x9A, 0xCF); // Code segment
        _gdt_set_gate(2, 0, 0xFFFF, 0x92, 0xCF); // Data segment
        _gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
        _gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment*/
						     
	_gdt_flush((uint32_t)&gdt_ptr);
}

void _gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	/* TODO: Add some sort of assert to ensure num < NUM_GDT_ENTRIES */

	gdt_entries[num].base_low    = (base & 0xFFFF);
        gdt_entries[num].base_middle = (base >> 16) & 0xFF;
        gdt_entries[num].base_high   = (base >> 24) & 0xFF;

        gdt_entries[num].limit_low   = (limit & 0xFFFF);
        gdt_entries[num].granularity = (limit >> 16) & 0x0F;

        gdt_entries[num].granularity |= gran & 0xF0;
        gdt_entries[num].access      = access;
}

