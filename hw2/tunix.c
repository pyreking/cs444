/*********************************************************************
*       file:           tunix.c
*       author:         Austin Guiney
*
*       trap handler for read, write, and exit systemcalls
*
*/

#include <stdio.h>
#include <cpu.h>
#include <gates.h>
#include "tsystm.h"
#include "tsyscall.h"

/* Trap handler for system calls. */
extern IntHandler syscall;
/* Assembly function that executes user mode code. */
extern void ustart(void);

/* Initialize the kernel for the operating system. */
void init_kernel()
{
  /* Initialize I/O operations. */
  ioinit();
  /* Set up the trap gate so that trap requests can be serviced. */
  set_trap_gate(0x80,&syscall);
  /* Call ustart() to start executing user mode code. */
  ustart();
}

/* Trap handler for syscall. */
void syscallc(int user_eax, int devcode, char *buf, int bufflen)
{
  /* Get the trap code from %eax. */
  int trap_code = user_eax;

  /* Perform the proper trap service routine based on the trap code. */
  switch (trap_code) {

    /* Perform a sysexit. */
    case EXIT_TRAP_NUMBER:
      sysexit(devcode);
      break;
    /* Perform a sysread. */
    case READ_TRAP_NUMBER:
      sysread(devcode, buf, bufflen);
      break;
    /* Perform a syswrite. */
    case WRITE_TRAP_NUMBER:
      syswrite(devcode, buf, bufflen);
      break;
    /* Print an error message if there is no valid trap code. */
    default:
      kprintf("Error: %d is not a valid trap code.", trap_code);
    }
}