#include "sched.h"
#include "proc.h"
#include <cpu.h>
#include <stdio.h>

#define BUFLEN 200

extern void asmswtch(PEntry *oldproc, PEntry *curproc);
extern void debug_log(char * msg);
void log_process_switch(PEntry *oldproc);

void schedule(void) {
  PEntry *oldproc = curproc;

  cli();

  for (int i = 1; i < NPROC; i++) {
    if (proctab[i].p_status == RUN) {
      curproc = &proctab[i];
      log_process_switch(oldproc);
      asmswtch(oldproc, curproc);
    }
  }
  
  sti();
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