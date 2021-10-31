# file: crt0.s
# Description: Enters into the main function in test1.c
# and then exits into tutor.
# Author: Austin Guiney

.global ustart
.text

ustart:
    movl $0x3ffff0, %esp    # Set user program stack
    movl $0, %ebp           # Make debugger backtrace terminate here
    call main               # Call into main function in test1.c.
    pushl %eax              # Push the exit code to the stack.
    call exit               # Call into the exit function.
