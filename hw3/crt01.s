.globl ustart1, main1, exit
.text
			
ustart1:	
        movl $0x3ffff0, %esp         # reinit the stack
		movl $0, %ebp                # and frame pointer
		call main1                   # call main in the uprog.c
        pushl %eax                   # push the retval=exit_code on stack
        call exit                    # call sysycall exit
               
