/*********************************************************************
*       Modified by Austin Guiney for CS444 class.
*       file:           tunix.c
*       date:           11/29/2021
*
*       Core kernel code.
*
*/

#include <cpu.h>
#include <gates.h>
#include "tsyscall.h"
#include "tsystm.h"
#include "proc.h"
#include "sched.h"
#include <string.h>

/* Symbolic constants for the delay function. */
#define MILLION 1000000
#define DELAY (400 * MILLION)

/* Main method for process 1. */
extern void ustart1(void);
/* Main method for process 2. */
extern void ustart2(void);
/* Main method for process 3. */
extern void ustart3(void);

/* Initializes the process table. */
void init_proctab(void);
/* Calls the scheduler if the processes are not zombies and shutdowns the OS afterwards. */
void process0(void);
/* Small delay used for draining output from the TX queue. */
void delay(void);

/* Initializes the process table variables defined in proc.h. */
PEntry proctab[NPROC], *curproc, *lastproc;

extern IntHandler syscall; /* the assembler envelope routine    */
extern void finale(void);
/* kprintf is proto'd in stdio.h, but we don't need that for anything else */
void kprintf(char *, ...);	

/* functions in this file */
void udebug_set_trap_gate(int n, IntHandler *inthand_addr, int debug);
void uset_trap_gate(int n, IntHandler *inthand_addr);
int sysexit(int);
void k_init(void);
void shutdown(void);
void syscallc(int user_eax, int devcode, char *buff , int bufflen);

/* Record debug info in otherwise free memory between program and stack */
/* 0x300000 = 3M, the start of the last M of user memory on the SAPC */
#define DEBUG_AREA 0x300000
char *debug_log_area = (char *)DEBUG_AREA;
char *debug_record;  /* current pointer into log area */ 

/* Keeps track of the number of zombie processes. */
int num_zombies;

#define MAX_CALL 6

/* optional part: syscall dispatch table */
static  struct sysent {
       short   sy_narg;        /* total number of arguments */
       int     (*sy_call)(int, ...);   /* handler */
} sysent[MAX_CALL];

/****************************************************************************/
/* k_init: this function for the initialize  of the kernel system*/
/****************************************************************************/

void k_init(){
  debug_record = debug_log_area; /* clear debug log */
  cli();
  ioinit();            /* initialize the deivce */ 
  uset_trap_gate(0x80, &syscall);   /* SET THE TRAP GATE*/

  /* Note: Could set these with array initializers */
  /* Need to cast function pointer type to keep ANSI C happy */
  sysent[TREAD].sy_call = (int (*)(int, ...))sysread;
  sysent[TWRITE].sy_call = (int (*)(int, ...))syswrite;
  sysent[TEXIT].sy_call = (int (*)(int, ...))sysexit;
 
  sysent[TEXIT].sy_narg = 1;    /* set the arg number of function */
  sysent[TREAD].sy_narg = 3;
  sysent[TIOCTL].sy_narg = 3;
  sysent[TWRITE].sy_narg = 3;
	sti(); /* user runs with interrupts on */
  /* Initializes the process table. */
	init_proctab();
  /* Call process 0. */
	process0();
}


/* Initializes the process table. */
void init_proctab() {
	int i;

  /* Set the SAVED_PC values for the three user processes. */
	proctab[1].p_savedregs[SAVED_PC] = (int) &ustart1;
	proctab[2].p_savedregs[SAVED_PC] = (int) &ustart2;
	proctab[3].p_savedregs[SAVED_PC] = (int) &ustart3;

  /* Set the SAVED_ESP values for the three user processes. */
	proctab[1].p_savedregs[SAVED_ESP] = ESP1;
	proctab[2].p_savedregs[SAVED_ESP] = ESP2;
	proctab[3].p_savedregs[SAVED_ESP] = ESP3;

  /* Set the SAVED_EFLAGS, SAVED_ESP and status values for process 0 and the
   * three user processes. */
	for (i = 0; i < NPROC; i++) {
		proctab[i].p_savedregs[SAVED_EFLAGS] = 0x1 << 9;
    proctab[i].p_savedregs[SAVED_EBP] = 0;
		proctab[i].p_status = RUN;
	}

  /* Set the current process to process 0. */
	curproc = &proctab[0];
  /* Set the last process to run to process 0. */
  lastproc = curproc;

  /* Set the number of zombies to 0. */
  num_zombies = 0;
}


/* shut the system down */
void shutdown()
{
  kprintf("SHUTTING THE SYSTEM DOWN!\n");
  kprintf("Debug log from run:\n");
  kprintf("Marking kernel events as follows:\n");
  kprintf("   *   TX interrupt occured.\n");
  kprintf("  [c]  Character put in TX queue.\n");
  kprintf("  >c   Character output from TX queue.\n");
  kprintf("%s", debug_log_area);	/* the debug log from memory */
  kprintf("\nLEAVE KERNEL!\n\n");
  finale();		/* trap to Tutor */
}

/****************************************************************************/
/* syscallc: this function for the C part of the 0x80 trap handler          */
/* OK to just switch on the system call number here                         */
/* By putting the return value of syswrite, etc. in user_eax, it gets       */
/* popped back in sysentry.s and returned to user in eax                    */
/****************************************************************************/

void syscallc( int user_eax, int devcode, char *buff , int bufflen)
{
  int nargs;
  int syscall_no = user_eax;

  switch(nargs = sysent[syscall_no].sy_narg)
    {
    case 1:         /* 1-argument system call */
	user_eax = sysent[syscall_no].sy_call(devcode);   /* sysexit */
	break;
    case 3:         /* 3-arg system call: calls sysread or syswrite */
	user_eax = sysent[syscall_no].sy_call(devcode,buff,bufflen); 
	break;
    default: kprintf("bad # syscall args %d, syscall #%d\n",
		     nargs, syscall_no);
    }
} 

/****************************************************************************/
/* sysexit: this function for the exit syscall fuction */
/****************************************************************************/

int sysexit(int exit_code){
  /* Disable interrupts because of critical region. */
  cli();
  /* Update the exit value of the zombie process. */
  curproc->p_exitval = exit_code;
  /* Set the status of the current process to ZOMBIE. */
  curproc->p_status = ZOMBIE;
  /* Increment the number of zombies. */
  num_zombies++;
  /* Print out the exit code of the zombie process. */
  kprintf("\n EXIT CODE IS %d\n", exit_code);
  /* Call the scheduler. */
  schedule();
	shutdown();  /* we have only one program here, so all done */
	return 0;    /* never happens, but keeps gcc happy */
}

/****************************************************************************/
/* uset_trap_gate: this function for setting the trap gate */
/****************************************************************************/
extern void locate_idt(unsigned int *limitp, char ** idtp);

void uset_trap_gate(int n, IntHandler *inthand_addr)
{
  udebug_set_trap_gate(n, inthand_addr, 0);
}

/* write the nth idt descriptor as a trap gate to inthand_addr */
void udebug_set_trap_gate(int n, IntHandler *inthand_addr, int debug)
{
  char *idt_addr;
  Gate_descriptor *idt, *desc;
  unsigned int limit = 0;

  if (debug)
    kprintf("Calling locate_idt to do sidt instruction...\n");
  locate_idt(&limit,&idt_addr);
  /* convert to CS seg offset, i.e., ordinary address, then to typed pointer */
  idt = (Gate_descriptor *)(idt_addr - KERNEL_BASE_LA);
  if (debug)
    kprintf("Found idt at %x, lim %x\n",idt, limit);
  desc = &idt[n];               /* select nth descriptor in idt table */
  /* fill in descriptor */
  if (debug)
    kprintf("Filling in desc at %x with addr %x\n",(unsigned int)desc,
           (unsigned int)inthand_addr);
  desc->selector = KERNEL_CS;   /* CS seg selector for int. handler */
  desc->addr_hi = ((unsigned int)inthand_addr)>>16; /* CS seg offset of inthand */
  desc->addr_lo = ((unsigned int)inthand_addr)&0xffff;
  desc->flags = GATE_P|GATE_DPL_KERNEL|GATE_TRAPGATE; /* valid, trap */
  desc->zero = 0;
}

/* append msg to memory log */
void debug_log(char *msg)
{
    strcpy(debug_record, msg);
    debug_record +=strlen(msg);
}

/* Calls the scheduler if the processes are not zombies and shutdowns the OS afterwards. */
void process0() {
  /* Continually call the scheduler until all of the processes are zombies. */
	while (num_zombies < (NPROC - 1)) {
	  schedule();
	}
  
  /* Drain the output from the TX queue. */
  delay();
  /* Shutdown the OS. */
	shutdown();
}

/* Small delay used for draining output from the TX queue. */
void delay() {
  int i;

  for (i = 0; i < DELAY; i++) {
  
  }
}