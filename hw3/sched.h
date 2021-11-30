/*********************************************************************
*       file:           sched.h
*       author:         Austin Guiney
*       date:           11/29/2021
*
*       Function prototypes for sched.c.
*
*/

#ifndef SCHED_H
#define SCHED_H
#include "proc.h"

/* Schedules processes in numerical order using a non-preemptive scheduler. */
void schedule(void);
/* Blocks the current process */
void sleep(WaitCode event);
/* Wakeup every blocked process that has the specified waitcode. */
void wakeup(WaitCode event);

#endif