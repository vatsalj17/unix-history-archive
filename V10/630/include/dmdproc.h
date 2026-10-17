/*       Copyright (c) 1987 AT&T   */
/*       All Rights Reserved       */

/*       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T   */
/*       The copyright notice above does not evidence any      */
/*       actual or intended publication of such source code.   */

#include <sccsid.h>
HVERSION(dmdproc, @(#)dmdproc.h	1.1.1.8	(5/10/87));

#ifndef PROC_H
#define PROC_H	1

#define	NPROC		8	/* caution - this is repeated in io.h -
					total xt connections to a host */
#define	STKSIZE		((unsigned long)2048) /* default per-process stack size */
#define MAGIC		0xaaaaaaaa  /* magic number for stack overflow check */
#define	CBSIZE		64	/* NPROC*CBSIZE <= queue.h/NCHARS */
#define IO_Q_SIZE	512	/* queue size interrupt handlers use */
#define RCVQBUFSIZE	(CBSIZE*3) /* size of per process receive queue */
#define KBDQBUFSIZE	128	/* size of kbdqueue */

typedef struct Queue {
	unsigned char *first;		/* position before first character */
	unsigned char *last;		/* pointer to last character */
	unsigned char *array;		/* address of start of circular buffer */
	unsigned char *array_last;	/* address of end of circular buffer */
	unsigned short c_cc;		/* characters currently in the queue */
	unsigned short size;		/* size of the queue */
	unsigned short ipl;		/* ipl for this queue - not currently used */
	long pad[2];			/* for the future */
} Queue;

typedef struct Timer {
	unsigned long waketime;	/* time to wake up */
	int (*tfire)();		/* control executes this when alarm fires */
	long targ;		/* unique argument to the above function */
	struct Timer *tnext;	/* links to the next Timer */
} Timer;

typedef struct Proc {
	long 	*sp;		/* stack pointer (really frame pointer) */
	char	*text;		/* address of text */
	char	*data;		/* address of data */
	char	*bss;		/* address of bss */
	long	state;		/* see definitions below */
	long	state2;		/* for the future */
	long	state3;		/* for the future */
	long	appl;		/* for applications to use (wproc uses it) */
	Rectangle uDrect;	/* application's Drect */
	Bitmap	*uJdisplayp;	/* application's display */
	int	uargc;		/* application's argc */
	char	**uargv;	/* application's argv */
	struct Mouse umouse;	/* application's mouse */
	Point	(*ctob)();	/* characters to bits, args are (x,y,P) */
	Point	(*btoc)();	/* bits to characters, args are (x,y,P) */
	long	pad1[8];	/* for the future */
	struct Logio *host;	/* logical host device for this proc */
	struct Layer *layer;	/* layer structure for this process */
	long    caches;		/* what caches this process has requested objects from */
	struct Msqid_q *msqid_q;/* link list of message queue id's */
	long	pad2[6];	/* for the future */
	Rectangle rect;		/* rectangle on screen (Drect) */
	long	pad3[4];	/* for the future */
	long	traploc;	/* for the debugger */
	short	traptype;	/* for the debugger */
	long	pad4[3];	/* more pad */
	Point	scurpt;		/* Current pt as offset from Drect.origin in
				   screen coordinates for printf, etc. */
	Point	curpt;		/* Current pt in layer coordinates for the
				   jline etc. crowd */
	Timer	timer;		/* timer for this process, used for alarm */
	long	pad5[4];	/* more pad */
	Texture16 *cursor;	/* mouse cursor for this process */
	short	inhibited;	/* stack for cursor tracking */
	long	pad6[12];	/* for the future */
	struct Proc *nextproc;	/* next process in the linked list */
	short	xtchan;		/* xt channel this process is connected to */
	Queue kbdqueue;		/* kbd queue for processes who requested kbd */
	long	pad7[12];	/* more pad */
	int	kbd_LEDs;	/* per process keyboard LED status */
	char    enterkey[6];    /* per process definition for the enter key */
	Queue	rcvq;		/* per process receive character queue */
	long    pad8[6];        /* for the future */
	int	*stack;	/* pointer to per process stack - int for alignment  */
	unsigned long stksize;	/* size of stack pointed to by stack  */
	long	pad9[12];	/* for the future */
	unsigned char kbdqueue_buf[KBDQBUFSIZE];  /* buffer for kbdqueue */
	unsigned char rcvq_buf[RCVQBUFSIZE];  /* buffer for rcvq */
} Proc;


#define PROCSTATES


#ifdef	MPXTERM
#define PROCSTATES
#endif
#ifdef PANDORA
#define PROCSTATES
#endif

#ifdef DMODEL
#define PROCSTATES
#endif

#ifdef PROCSTATES
/*
 * Data at start of user program.
 * This has been put into the Proc structure.
 *
 *struct udata {
 *	Rectangle Drect;
 *	struct Layer * Jdisplayp;
 *	int argc;
 *	char **argv;
 *	struct Mouse mouse;
 *};
 */

/*
 * if(P->host == LOCAL) process has no host connection
*/
#define LOCAL	(struct Logio *)0

/*
 * states
*/
#define	RUN		0x1	  /* ready to be scheduled */
#define	OKTOUNBLK	0x2	  /* ready to unblock a BLOCKED process */
#define	BLOCKED		0x4	  /* blocked by user with ^S */
#define	USER		0x8	  /* a user-32ld'd process */
#define	KBDLOCAL	0x10	  /* has requested the KBD */
#define	MOUSELOCAL	0x20	  /* has requested the MOUSE */
#define	GOTMOUSE	0x40	  /* currently owns MOUSE */
#define	WAKEUP		0x80	  /* indicate an alarm has fired */
#define	MOVED		0x100	  /* layer got moved */
#define	UNBLOCKED	0x200	  /* Has been unblocked */
#define	ZOMBIE		0x400	  /* proc died horribly; waiting for debugger */
#define	RESHAPED	0x800	  /* layer got reshaped */
#define	ZOMBOOT		0x1000	  /* put in ZOMBIE state after booting */
#define	ALARMREQD	0x2000	  /* has requested an alarm */
#define LOCKLAYER	0x4000    /* can't delete layer with mouse */
#define NOPFEXPAND	0x8000    /* do not expand the PF keys */
#define NOPADEXPAND	0x10000   /* do not expand right key pad */
#define NOCURSEXPAND	0x20000   /* do not expand arrow keys and HOME key */
#define NOTRANSLATE	0x40000   /* send keyboard raw codes to process */
#define SCR_LOCK	0x80000   /* process is scroll locked */
#define LABEL		0x100000  /* process has a label area */
#define NO_RESHAPE	0x200000  /* window is not reshapable */
#define CURSXY_ONLY	0x400000  /* mouse move along xy axes only */
#define RESHAPE_OK 	0x800000  /* reshape to fixed size ok */
#define MODCURRENT	0x1000000 /* current state gets changed */
#define SCRLOCKREQD	0x2000000 /* do scroll lock led and label bar */
#define USERDELETE      0x4000000 /* user tried to delete LOCKLAYER'ed layer */
#define DONTCRASH	0x8000000 /* don't generate an exeception message -
				     just zombie process - used by dmdpi agent */
#define MSG_WAKE	0x10000000/* message status has changed */

#define	setrun(p)	((p)->state|=RUN)
#define	setnorun(p)	((p)->state&=~RUN)

#else
#define	MOVED		0x100	  /* layer got moved */
#define	RESHAPED	0x800	  /* layer got reshaped */
#define LOCKLAYER	0x4000    /* can't delete layer with mouse */
#define NOPFEXPAND	0x8000    /* do not expand the PF keys */
#define NOPADEXPAND	0x10000   /* do not expand right key pad */
#define NOCURSEXPAND	0x20000   /* do not expand arrow keys and HOME key */
#define NOTRANSLATE	0x40000   /* send keyboard raw codes to process */
#define SCR_LOCK	0x80000   /* process is scroll locked */
#define SCRLOCKREQD	0x2000000 /* do scroll lock led and label bar */
#define USERDELETE      0x4000000 /* user tried to delete LOCKLAYER'ed layer */
#endif	/* PROCSTATES */


#define INSET	4
#define NULLP	(Proc *)0
#define PROCSIZ  sizeof (Proc)
#endif
