.section .text

.global load_page_dir
load_page_dir:
	push %ebp
	mov %esp, %ebp
	mov 8(%esp), %eax
	mov %eax, %cr3
	mov %ebp, %esp
	pop %ebp
	ret

.global enable_paging
enable_paging:
	push %ebp
	mov %esp, %ebp
	mov %cr0, %eax
	or $0x80000001, %eax
	mov %eax, %cr0
	mov %ebp, %esp
	pop %ebp
	ret




