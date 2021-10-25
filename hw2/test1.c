#include <stdio.h>
#include "tty_public.h"
#include "tsystm.h"
#include "tunistd.h"
#define BUFLEN 20

int main(void)
{
  char buf[BUFLEN];
  int got;

  kprintf("Trying to get a minimum viable setup.\n");
  kprintf("Trying simple write(4 chars)...\n");
  got = syswrite(TTY1,"hi!\n",4);
  kprintf("write of 4 returned %d\n",got);
}