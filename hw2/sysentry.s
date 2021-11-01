# file: sysentry.s
# Description: Enters into the trap handler in tunix.c.
# Modified by Austin Guiney for CS444 class.
.globl syscallc, syscall

syscall:
    pushl %edx    # push the values of eax to edx to stack
    pushl %ecx
    pushl %ebx
    pushl %eax
    call syscallc # call c trap routine in tunix.c
    addl $4, %esp # use the new value of %eax after the system call
    popl %ebx
    popl %ecx
    popl %edx
    iret          # return to caller
