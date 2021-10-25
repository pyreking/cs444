.globl syscallc, syscall

syscall:
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax
    call syscallc
    popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    iret
