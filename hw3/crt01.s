/*********************************************************************
*       file:           crt01.s
*       author:         Austin Guiney
*       date:           11/29/2021
*
*       Starts process 1 and then exits.
*
*/

.globl ustart1, main1, exit
.text
			
ustart1:	
        movl $0x280000, %esp         # reinit the stack
		movl $0, %ebp                # and frame pointer
		call main1                   # call main in the uprog.c
        pushl %eax                   # push the retval=exit_code on stack
        call exit                    # call sysycall exit
               
