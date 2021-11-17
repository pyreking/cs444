.globl ustart3, main3, exit
.text
			
ustart3:	
        movl $0x3ffff0, %esp         # reinit the stack
		movl $0, %ebp                # and frame pointer
		call main3                   # call main in the uprog.c
        pushl %eax                   # push the retval=exit_code on stack
        call exit                    # call sysycall exit
               