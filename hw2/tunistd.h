#ifndef TUNISTD_H
#define TUNISTD_H

int write(int dev, char *buf, int nchar);
int read(int dev, char *buf, int nchar);
int exit(int exitcode);

#endif