.section .text
.extern entry_point
.global _start
_start:
    xor %ebp, %ebp
    popq %rdi
    mov %rsp, %rsi
    and $~0xf, %rsp
    pushq %rsp
    pushq $0
    call entry_point
    mov $231, %eax
    xor %edi, %edi
    syscall

