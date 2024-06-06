# icix

TODO: 

 - How to check whether code to detect spurious IRQs works?
 - Manually save the registers in the ISR handler instead of pushad/popap.
 - in irq handler (pic) Im not changing the data segment register to kernel mode (0x10), is that an issue? Im assuming paging will anyway deal with required protection
 - proper exception handlers
 - in IDT we are not setting proper ring levels for interrupts (mainly 0x20 onwards ig)

