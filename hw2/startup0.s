# asm startup file
# very first module in load

.globl _start, _startupc, shutdown

_start:     movl $0x3ffff0, %esp   # set user program stack
            movl $0, %ebp          # make debugger backtrace terminate here
            call _startupc         # call C startup, which calls user main
shutdown:   int $3                 # return to Tutor


