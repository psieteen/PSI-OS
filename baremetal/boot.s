.section .text
.global _start

_start:
    ldr x0, =_stack_top
    mov sp, x0
    bl kernel_main

hang:
    wfe
    b hang

.section .bss
.align 4
_stack_bottom:
    .space 16384
_stack_top:
