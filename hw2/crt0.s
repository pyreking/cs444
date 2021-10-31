.global ustart
.text

ustart:
    movl $0x3ffff0, %esp
    movl $0, %ebp
    call main
    pushl %eax
    call exit
