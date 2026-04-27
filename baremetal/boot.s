.section .text
.global _start

_start:
    // Set up stack pointer
    ldr x0, =_stack_top
    mov sp, x0
    
    // Call the kernel's main function
    bl kernel_main

// If kernel_main ever returns, halt forever
hang:
    wfe
    b hang

// Reserve space for stack
.section .bss
.align 4
_stack_bottom:
    .space 16384    // 16KB stack
_stack_top:
