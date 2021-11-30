/*********************************************************************
*       file:           crt01.s
*       author:         Austin Guiney
*       date:           11/29/2021
*
*       Starts process 3 and then exits.
*
*/

.globl ustart3, main3, exit
.text
			
ustart3:	
        movl $0x2a0000, %esp         # reinit the stack
		movl $0, %ebp                # and frame pointer
		call main3                   # call main in the uprog.c
        pushl %eax                   # push the retval=exit_code on stack
        call exit                    # call sysycall exit
               