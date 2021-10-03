/*********************************************************************
*
*       file:           tty.c
*       author:         betty o'neil
*
*       tty driver--device-specific routines for ttys 
*
*/
#include <stdio.h>  /* for kprintf prototype */
#include <serial.h>
#include <cpu.h>
#include <pic.h>
#include "ioconf.h"
#include "tty_public.h"
#include "tty.h"
#include "queue/queue.h"

struct tty ttytab[NTTYS];        /* software params/data for each SLU dev */

/* Record debug info in otherwise free memory between program and stack */
/* 0x300000 = 3M, the start of the last M of user memory on the SAPC */
#define DEBUG_AREA 0x300000
#define BUFLEN 20

char *debug_log_area = (char *)DEBUG_AREA;
char *debug_record;  /* current pointer into log area */ 

/* tell C about the assembler shell routines */
extern void irq3inthand(void), irq4inthand(void);

/* C part of interrupt handlers--specific names called by the assembler code */
extern void irq3inthandc(void), irq4inthandc(void); 

/* the common code for the two interrupt handlers */                           static void irqinthandc(int dev);

/* prototype for debug_log */ 
void debug_log(char *);

/*====================================================================
*
*       tty specific initialization routine for COM devices
*
*/

void ttyinit(int dev)
{
  int baseport;
  struct tty *tty;		/* ptr to tty software params/data block */

  debug_record = debug_log_area; /* clear debug log */
  baseport = devtab[dev].dvbaseport; /* pick up hardware addr */
  tty = (struct tty *)devtab[dev].dvdata; /* and software params struct */

  if (baseport == COM1_BASE) {
      /* arm interrupts by installing int vec */
      set_intr_gate(COM1_IRQ+IRQ_TO_INT_N_SHIFT, &irq4inthand);
      pic_enable_irq(COM1_IRQ);
  } else if (baseport == COM2_BASE) {
      /* arm interrupts by installing int vec */
      set_intr_gate(COM2_IRQ+IRQ_TO_INT_N_SHIFT, &irq3inthand);
      pic_enable_irq(COM2_IRQ);
  } else {
      kprintf("Bad TTY device table entry, dev %d\n", dev);
      return;			/* give up */
  }
  tty->echoflag = 1;		/* default to echoing */
  tty->rin = 0;               /* initialize indices */
  tty->rout = 0;
  tty->rnum = 0;              /* initialize counter */
  tty->tin = 0;               /* initialize indices */
  tty->tout = 0;
  tty->tnum = 0;              /* initialize counter */

  init_queue(&(tty->read_queue), MAXBUF);
  init_queue(&(tty->write_queue), MAXBUF);
  init_queue(&(tty->echo_queue), MAXBUF);

  /* enable interrupts on receiver */
  outpt(baseport+UART_IER, UART_IER_RDI); /* RDI = receiver data int */
}


/*====================================================================
*
*       Useful function when emptying/filling the read/write buffers
*
*/

#define min(x,y) (x < y ? x : y)


/*====================================================================
*
*       tty-specific read routine for TTY devices
*
*/

int ttyread(int dev, char *buf, int nchar)
{
  int baseport;
  struct tty *tty;
  int i = 0;

  char log[BUFLEN];
  int saved_eflags;        /* old cpu control/status reg, so can restore it */

  baseport = devtab[dev].dvbaseport; /* hardware addr from devtab */
  tty = (struct tty *)devtab[dev].dvdata;   /* software data for line */

  while (i < nchar) {
    saved_eflags = get_eflags();
    cli();			/* disable ints in CPU */

    if (queuecount(&(tty->read_queue)) != 0) {
      buf[i] = dequeue(&(tty->read_queue));      /* copy from ibuf to user buf */

      sprintf(log, ">%c", buf[i]);
      debug_log(log);
      i++;
    }

    set_eflags(saved_eflags);     /* back to previous CPU int. status */
  }

  return nchar;       /* but should wait for rest of nchar chars if nec. */
  /* this is something for you to correct */
}


/*====================================================================
*
*       tty-specific write routine for SAPC devices
*       (cs444: note that the buffer tbuf is superfluous in this code, but
*        it still gives you a hint as to what needs to be done for
*        the interrupt-driven case)
*
*/

int ttywrite(int dev, char *buf, int nchar)
{
  int baseport;
  struct tty *tty;
  int i = 0;
  char log[BUFLEN];
  int saved_eflags;

  baseport = devtab[dev].dvbaseport; /* hardware addr from devtab */
  tty = (struct tty *)devtab[dev].dvdata;   /* software data for line */

  saved_eflags = get_eflags();
  cli();

  while (i < nchar && queuecount(&(tty->write_queue)) < MAXBUF) {
    enqueue(&(tty->write_queue), buf[i]);
    sprintf(log,"<%c", buf[i]); /* record input char-- */
    debug_log(log);
    i++;
  }

  outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
  set_eflags(saved_eflags);

  while (i < nchar) {
    saved_eflags = get_eflags();
    cli();			/* disable ints in CPU */

    if (queuecount(&(tty->write_queue)) < MAXBUF) {
      enqueue(&(tty->write_queue), buf[i]);
      sprintf(log,"<%c", buf[i]); /* record input char-- */
      debug_log(log);
      i++;
      outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
    }

    set_eflags(saved_eflags); 
  }

  return nchar;
}

/*====================================================================
*
*       tty-specific control routine for TTY devices
*
*/

int ttycontrol(int dev, int fncode, int val)
{
  struct tty *this_tty = (struct tty *)(devtab[dev].dvdata);

  if (fncode == ECHOCONTROL)
    this_tty->echoflag = val;
  else return -1;
  return 0;
}



/*====================================================================
*
*       tty-specific interrupt routine for COM ports
*
*   Since interrupt handlers don't have parameters, we have two different
*   handlers.  However, all the common code has been placed in a helper 
*   function.
*/
  
void irq4inthandc()
{
  irqinthandc(TTY0);
}                              
  
void irq3inthandc()
{
  irqinthandc(TTY1);
}                              

void irqinthandc(int dev){  
  int ch, iir;
  struct tty *tty = (struct tty *)(devtab[dev].dvdata);
  int baseport = devtab[dev].dvbaseport; /* hardware i/o port */;

  pic_end_int();                /* notify PIC that its part is done */
  debug_log("*");
  iir = inpt(baseport+UART_IIR);

  switch (iir & UART_IIR_ID) {
    case UART_IIR_RDI:
      ch = inpt(baseport+UART_RX);	/* read char, ack the device */

      if (queuecount(&(tty->read_queue)) < MAXBUF) {   /* if space left in ring buffer */
        enqueue(&(tty->read_queue), ch); /* put char in ibuf, step ptr */
      }

      if (tty->echoflag) {       /* if echoing wanted */
        enqueue(&(tty->echo_queue), ch);
        outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);   /* echo char: see note above */
      }

    case UART_IIR_THRI:
      if (queuecount(&(tty->echo_queue)) != 0) {
        outpt(baseport+UART_TX, dequeue(&(tty->echo_queue)));
      } else if (queuecount(&(tty->write_queue)) != 0) {
          ch = dequeue(&(tty->write_queue));
          outpt(baseport+UART_TX, ch);
      } else {
          outpt(baseport+UART_IER, UART_IER_RDI);
      }
  }
}

/* append msg to memory log */
void debug_log(char *msg)
{
    strcpy(debug_record, msg);
    debug_record +=strlen(msg);
}
