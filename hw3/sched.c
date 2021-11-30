/*********************************************************************
*       file:           sched.c
*       author:         Austin Guiney
*       date:           11/29/2021
*
*       A non-preemptive scheduler for the tunix kernel.
*
*/

#include <stdio.h>
#include "sched.h"
#include "proc.h"
#include <cpu.h>
/* The length of the buffer used in log_process_switch. */
#define BUFLEN 200

/* Switches currently running process from oldproc to curproc */
extern void asmswtch(PEntry *oldproc, PEntry *curproc);
extern void debug_log(char * msg);
/* Saves a history of every process switch in the debug log. */
void log_process_switch(PEntry *oldproc);

/* Schedules processes in numerical order using a non-preemptive scheduler. */
void schedule(void) {
  /* Save the current process to oldproc. */
  PEntry *oldproc = curproc;
  int saved_eflags;

  /* Loop through every process. */
  for (int i = 1; i < NPROC; i++) {
    /* Run the process if the process is availible to run. Only run processes that are not
     * the current process and were not the last process to run successfully. */
    if (proctab[i].p_status == RUN && &proctab[i] != curproc && &proctab[i] != lastproc) {
      /* Save the eflags of the currently running process. */
      saved_eflags = get_eflags();
      /* Disable interrupts because of critical region. */
      cli();

      /* Update the current process. */
      curproc = &proctab[i];
      /* Update the variable that tracks the last process that was run. */
      lastproc = oldproc;
      
      /* Saves a log of the process switch in the debug log. */
      log_process_switch(oldproc);

      /* Switch from the old process to the new process. */
      asmswtch(oldproc, curproc);

      /* Enable interrupts by restoring the eflags register. */
      set_eflags(saved_eflags);
    }
  }

  /* If there are no more availible processes to run, run process 0 if it is not already running. */
  if ((proctab[1].p_status != RUN) && (proctab[2].p_status != RUN) && (proctab[3].p_status != RUN)
      && curproc != &proctab[0]) {
    /* Save the eflags of the currently running process. */
    saved_eflags = get_eflags();
    /* Disable interrupts because of critical region. */
    cli();

    /* Update the current process. */
    curproc = &proctab[0];

    /* Saves a log of the process switch in the debug log. */
    log_process_switch(oldproc);

    /* Switch from the old process to the new process. */
    asmswtch(oldproc, curproc);
    
    /* Enable interrupts by restoring the eflags register. */
    set_eflags(saved_eflags);
  }
}

/* Blocks the current process */
void sleep(WaitCode event) {
  /* Disable interrupts because of critical region. */
  cli();

  /* Update the status of the current process to BLOCKED. */
  curproc->p_status = BLOCKED;
  /* Update the waitcode for the current process. */
  curproc->p_waitcode = event;

  schedule();

  /* Enable interrupts again. */
  sti();
}

/* Wakeup every blocked process that has the specified waitcode. */
void wakeup(WaitCode event) {
  int i;

  /* Loop through every process in the process table. */
  for (i = 1; i < NPROC; i++) {
    /* If the process has the same waitcode and is blocked, unblock it by setting
     * its status to RUN. */
    if (proctab[i].p_waitcode == event && proctab[i].p_status == BLOCKED) {
      proctab[i].p_status = RUN;
    }
  }

}

/* Saves a history of every process switch in the debug log. */
void log_process_switch(PEntry *oldproc) {
  /* The buffer for the log of the process switch. */
  char buf[BUFLEN];

  /* Check the status of the last process to run */
  switch (oldproc->p_status) {
    /* Log the process switch if it is a ZOMBIE. */
    case ZOMBIE:
      sprintf(buf, "|(%dz-%d)", oldproc - proctab, curproc - proctab);
      break;
    /* Log the process switch if it is BLOCKED. */
    case BLOCKED:
      sprintf(buf, "|(%db-%d)", oldproc - proctab, curproc - proctab);
      break;
    /* Log the process switch if it is not a ZOMBIE or BLOCKED. */
    default:
      sprintf(buf, "|(%d-%d)", oldproc - proctab, curproc - proctab);
      break;
  }

  /* Put the log of the process switch in the debug log if the buffer is nonempty. */
  if (strlen(buf) > 0) {
      debug_log(buf);
  }
}