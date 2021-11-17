#include "sched.h"
#include "proc.h"
extern void ustart1(void);
extern void ustart2(void);
extern void ustart3(void);

void schedule(void) {
  for (int i = 1; i < NPROC; i++) {
    if (proctab[i].p_status == RUN) {
      if (i == 1) {
        ustart1();
      }
      if (i == 2) {
        ustart2();
      }
      if (i == 3) {
        ustart3();
      }
    }
  }
}