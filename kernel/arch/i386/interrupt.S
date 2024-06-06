.altmacro
.section .text

.macro isr_err_stub idx:req
.global isr_\idx
isr_\idx:
	push $\idx
	jmp isr_common_stub
.endm

.macro isr_no_err_stub idx:req
.global isr_\idx
isr_\idx:
	push $0
	push $\idx
	jmp isr_common_stub
.endm

isr_common_stub:
	pushal
	mov %cr2, %eax
	push %eax

	call interrupt_handler

	addl $4, %esp # cr2
	popal

	addl $8, %esp # for int no., ecode
	iret

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

.macro isr_tmp1 from=32, to=255
isr_no_err_stub \from
.if \to-\from
isr_tmp1 %(\from+1),\to
.endif
.endm

isr_tmp1 32, 99
isr_tmp1 100, 199
isr_tmp1 200, 255

.section .data

.macro isr_stub_tmp from=0, to=255
	.long isr_\from
.if \to-\from
isr_stub_tmp %(\from+1),\to
.endif
.endm

.global isr_stub_table
isr_stub_table:
	isr_stub_tmp 0, 99
	isr_stub_tmp 100, 199
	isr_stub_tmp 200, 255

