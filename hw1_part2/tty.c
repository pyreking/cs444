/*********************************************************************
*       Modified by Austin Guiney for CS444 class.
*       File:           tty.c
*       Date:           10/12/21
*       Author:         betty o'neil
*       
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
  tty->echoflag = 1;          /* default to echoing */
  tty->rin = 0;               /* initialize indices */
  tty->rout = 0;
  tty->rnum = 0;              /* initialize counter */
  tty->tin = 0;               /* initialize indices */
  tty->tout = 0;
  tty->tnum = 0;              /* initialize counter */

  /* Initialize queeues for read, write, and echo functions. */
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

  /* Wait for the comport to read nchar characters before exiting */
  while (i < nchar) {
    /* Save context before disabling interrupts */
    saved_eflags = get_eflags();
    cli();			/* disable ints in CPU */

    /* Save the top character from the read queue to the user buffer. */
    if (queuecount(&(tty->read_queue)) != 0) {
      buf[i] = dequeue(&(tty->read_queue));      /* copy from ibuf to user buf */

      /* Log characters for debug output */
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
  
  /* Save context before disabling interrupts */
  saved_eflags = get_eflags();
  /* Disable interrupts while putting characters into the write queue */
  cli();

  /* Fill up the write queue to its max size before writing the characters. */
  while (i < nchar && queuecount(&(tty->write_queue)) < MAXBUF) {
    enqueue(&(tty->write_queue), buf[i]);
    sprintf(log,"<%c", buf[i]); /* record input char-- */
    debug_log(log);
    i++;
  }

  /* Enable interrupts for the RDI and THRI registers */
  outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
  /* Enable interrupts again by restoring the eflags register */
  set_eflags(saved_eflags);

  /* Wait for the comport to write nchar characters before exiting */
  while (i < nchar) {
    /* Save context before disabling interrupts */
    saved_eflags = get_eflags();
    cli();			/* disable ints in CPU */

    /* Put a character into the write queue if there is room for it */
    if (queuecount(&(tty->write_queue)) < MAXBUF) {
      enqueue(&(tty->write_queue), buf[i]);
      sprintf(log,"<%c", buf[i]); /* record input char-- */
      debug_log(log);
      i++;
      /* Reset the interrupt signals */
      outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
    }

    /* Enable interrupts again by restoring the eflags register */
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

/* ISR for read and write functions */
void irqinthandc(int dev){  
  int ch, iir;
  struct tty *tty = (struct tty *)(devtab[dev].dvdata);
  int baseport = devtab[dev].dvbaseport; /* hardware i/o port */;

  pic_end_int();                /* notify PIC that its part is done */
  debug_log("*");

  /* Get the status of the IIR register */
  iir = inpt(baseport+UART_IIR);

  /* Mask out the IIR ID to determine which function is ready */
  switch (iir & UART_IIR_ID) {
    /* Handle the ISR for the read function */
    case UART_IIR_RDI:
      ch = inpt(baseport+UART_RX);	/* read char, ack the device */

      /* Put a character in the read queue if there is room for it. */
      if (queuecount(&(tty->read_queue)) < MAXBUF) {
        enqueue(&(tty->read_queue), ch);
      }

      /* Put a character in the echo queue if the echo flag is enabled */
      if (tty->echoflag) {
        enqueue(&(tty->echo_queue), ch);
        /* Reset the interrupt signals */
        outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
      }

    /* Handle the ISR for the write function */
    case UART_IIR_THRI:
      /* Output an echo character if the echo queue isn't empty */
      if (queuecount(&(tty->echo_queue)) != 0) {
        outpt(baseport+UART_TX, dequeue(&(tty->echo_queue)));
        /* Write the character that is ready to be written */
      } else if (queuecount(&(tty->write_queue)) != 0) {
          ch = dequeue(&(tty->write_queue));
          outpt(baseport+UART_TX, ch);
      } else {
          /* Turn off the THRI interrupt signal after transmission is complete. */
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
