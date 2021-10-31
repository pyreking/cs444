/*********************************************************************
*       Modified by Austin Guiney for CS444 class.
*       file:           tsystm.h
*       author:         betty o'neil
*
*       kernel header for internal prototypes
*/

#ifndef TSYSTM_H
#define TSYSTM_H

/* initialize io package*/
void ioinit(void);
/* read nchar bytes into buf from dev */
int sysread(int dev, char *buf, int nchar);
/* write nchar bytes from buf to dev */
int syswrite(int dev, char *buf, int nchar);
/* misc. device functions */
int syscontrol(int dev, int fncode, int val);
/* exit to tutor from dev */
int sysexit(int exitcode);
/* debugging support */
void debug_log(char *msg);

#endif
