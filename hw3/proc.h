/*********************************************************************
*       Modified by Austin Guiney for CS444 class.
*       Date:        11/29/2021
*       file:        proc.h
*       
*       Process structure info.
*
*/
#ifndef PROC_H
#define PROC_H

/* number of processes: 3 user processes, 1 proc0 */
#define NPROC 4

/* Process 1 stack address */
#define ESP1 0x280000
/* Process 2 stack address */
#define ESP2 0x290000
/* Process 3 stack address */
#define ESP3 0x2a0000

#define  N_SAVED_REGS 7
/* 7 non-scratch CPU registers, saved in this order: */
enum cpu_regs {SAVED_ESP, SAVED_EBX, SAVED_ESI, SAVED_EDI,
	       SAVED_EBP, SAVED_EFLAGS, SAVED_PC};

/* for p_status ( RUN=1, BLOCKED=2, ZOMBIE=3) */
typedef enum proc_status {RUN = 1, BLOCKED, ZOMBIE} ProcStatus;

/* for p_waitcode, what the process is waiting for */
typedef enum proc_wait {TTY0_OUTPUT = 1, TTY1_OUTPUT} WaitCode;

/* Process Entry */
typedef struct {
   int p_savedregs[N_SAVED_REGS]; /* saved non-scratch registers */
   ProcStatus p_status;   /* RUN, BLOCKED, or ZOMBIE */
   WaitCode p_waitcode;   /* valid only if status==BLOCKED: TTY0_OUT, etc. */
   int p_exitval;         /* valid only if status==ZOMBIE */
} PEntry;

/* extern these globals here, define them in tunix.c
 * lastproc keeps track of the last process that was run. */
extern PEntry proctab[], *curproc, *lastproc;

#endif /*PROC_H */
