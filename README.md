# icix

Yet another hobby os. 

![Screenshot from 2024-06-28 22-34-25](https://github.com/double-fault/icix/assets/12422882/46e8fc2f-3d6e-4447-b6e6-25cff8fa2858)

Only supported architecture currently is x86. Use `build.sh` to build, and `qemu.sh` to run in QEMU (cross-compiler based on osdev wiki). Features:

 - [x] Booting up from any multiboot compliant bootloader
 - [x] Setting up the Global Descriptor Table, Interrupt Descriptor Table
 - [x] Setting up the Programmable Interrupt Controller
 - [x] Programmable Interrupt Timer
 - [x] Page frame allocator
 - [x] Virtual Memory Manager (paging)
 - [ ] Kernel Heap
 - [x] standard libc functions
 - [x] Unix file system, disk driver abstraction
 - [x] initrd driver
 - [ ] floppy disk driver
 - [ ] multitasking, user mode

Project structure is inspired from osdev wiki's [Meaty Skeleton](https://wiki.osdev.org/Meaty_Skeleton). Many aspects of the OS are based off of osdev wiki's articles or other operating systems (MIT's xv6, brokenthorn osdev series, James Molloy, etc.). 

# Todo

 - How to check whether code to detect spurious IRQs works?
 - Manually save the registers in the ISR handler instead of pushad/popap.
 - in irq handler (pic) Im not changing the data segment register to kernel mode (0x10), is that an issue? wrt protection
 - in IDT we are not setting proper ring levels for interrupts (mainly 0x20 onwards ig)
 - very weird bug: in printf trying to print 4096 (%d) only prints 40, not reproduced with other numbers yet..

