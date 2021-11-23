#ifndef SCHED_H
#define SCHED_H

#include "proc.h"

void schedule(void);
void sleep(WaitCode event);
void wakeup(WaitCode event);

#endif