// constants for multiboot loader
.set ALIGN,      1<<0
.set MEMINFO,    1<<1
.set VIDEOMODE,   1<<2
.set FLAGS,     ALIGN | MEMINFO | VIDEOMODE
.set MAGIC,     0x1BADB002
.set CHECKSUM,  -(MAGIC + FLAGS)

// multiboot header that marks the program as kerel (standard values)
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.long 0, 0, 0, 0, 0
.long 0
.long 1024, 768, 32

// allocate room for small stack
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

// _start as the entry point to the kernel
.section .text
.global _start
.type _start, @function
_start:
    // stack setup
    mov $stack_top, %esp

    push %ebx

    call kernel_main

    // disable interrupts
    cli
    // wait for next interrupt to arrive, locking the pc
1:  hlt
    // if it wakes up, return to halting
    jmp 1b

// set size of the _start symbol to the current location '.' minus the _start
// useful for debugging
.size _start, . - _start