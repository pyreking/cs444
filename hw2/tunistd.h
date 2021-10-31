/*********************************************************************
*       file:           tunistd.h
*       author:         Austin Guiney
*
*       header file for write, read, and exit system calls
*
*/


#ifndef TUNISTD_H
#define TUNISTD_H

int write(int dev, char *buf, int nchar);
int read(int dev, char *buf, int nchar);
int exit(int exitcode);

#endif