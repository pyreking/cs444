/*********************************************************************
*       Modified by Austin Guiney for CS444 class.
*       file:           uprog2.c
*       date:           11/29/2021
*
*       Main method for process 2.
*
*/

#include "tunistd.h"
#include "tty_public.h"

int main2()
{
  write(TTY1,"bbbbbbbbbb",10);
  return 4;
}
