#include <stdio.h>
#include "sched.h"
#include "proc.h"
#include <cpu.h>

#define BUFLEN 200

extern void asmswtch(PEntry *oldproc, PEntry *curproc);
extern void debug_log(char * msg);
void log_process_switch(PEntry *oldproc);

void schedule(void) {
  PEntry *oldproc = curproc;
  int saved_eflags;

  for (int i = 1; i < NPROC; i++) {
    if (proctab[i].p_status == RUN && &proctab[i] != curproc && &proctab[i] != lastproc) {
      saved_eflags = get_eflags();
      cli();

      curproc = &proctab[i];
      lastproc = oldproc;
      
      log_process_switch(oldproc);

      asmswtch(oldproc, curproc);
      set_eflags(saved_eflags);
    }
  }


  if ((proctab[1].p_status != RUN) && (proctab[2].p_status != RUN) && (proctab[3].p_status != RUN)
      && curproc != &proctab[0]) {
    saved_eflags = get_eflags();
    cli();

    curproc = &proctab[0];
    log_process_switch(oldproc);
    asmswtch(oldproc, curproc);
    
    set_eflags(saved_eflags);
  }
}

void sleep(WaitCode event) {
  cli();

  curproc->p_status = BLOCKED;
  curproc->p_waitcode = event;

  schedule();

  sti();
}

void wakeup(WaitCode event) {
  int i;

  for (i = 1; i < NPROC; i++) {
    if (proctab[i].p_waitcode == event && proctab[i].p_status == BLOCKED) {
      proctab[i].p_status = RUN;
      char bbuf[200];
      //sprintf(bbuf, "  woke up process %d   ", i);
      //debug_log(bbuf);
    }
  }

}

void log_process_switch(PEntry *oldproc) {
  char buf[BUFLEN];

  switch (oldproc->p_status) {
    case ZOMBIE:
      sprintf(buf, "|(%dz-%d)", oldproc - proctab, curproc - proctab);
      break;

    case BLOCKED:
      sprintf(buf, "|(%db-%d)", oldproc - proctab, curproc - proctab);
      break;

    default:
      sprintf(buf, "|(%d-%d)", oldproc - proctab, curproc - proctab);
      break;
  }

  if (strlen(buf) > 0) {
      debug_log(buf);
  }
}