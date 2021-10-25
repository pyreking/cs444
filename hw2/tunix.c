#include <cpu.h>
#include <gates.h>
#include "tsystm.h"

extern IntHandler syscall;
extern void ioinit(void);
extern void main(void);

void init_kernel()
{
  ioinit();
  set_trap_gate(0x80,&syscall);
  (void)main();
}

void syscallc(int user_eax, int devcode, char *buf, int bufflen)
{
  int trap_code = user_eax;

  switch (trap_code) {
    case 4:
      syswrite(devcode, buf, bufflen);
      break;
  }
}