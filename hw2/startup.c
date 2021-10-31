/*********************************************************************
*       file:           startup.c
*       author:         Austin Guiney
*
*       C startup file, called from startup0.s
*/

extern void clr_bss(void);
extern void init_devio(void);
extern void init_kernel(void);

void _startupc()
{
  clr_bss();			        /* clear BSS area (uninitialized data) */
  init_devio();			      /* latch onto Tutor-supplied info, code */
  init_kernel();			    /* execute user-supplied main */
}