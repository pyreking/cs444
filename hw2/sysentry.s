.globl syscallc, syscall

syscall:
    pushl %edx    # push the values of eax to edx to stack
    pushl %ecx
    pushl %ebx
    pushl %eax
    call syscallc # call c trap routine in tunix.c
    popl %eax     # pop the values of eax to edx from stack
    popl %ebx
    popl %ecx
    popl %edx
    iret          # return to caller
