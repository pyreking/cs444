.globl ustart2, main2, exit
.text
			
ustart2:	
        movl $0x290000, %esp         # reinit the stack
		movl $0, %ebp                # and frame pointer
		call main2                   # call main in the uprog.c
        pushl %eax                   # push the retval=exit_code on stack
        call exit                    # call sysycall exit
               