/*
 * conf/trace.c
 *
 * trace device
 *	Used for buffering the output of cmn_err ().
 */

/*
 * -----------------------------------------------------------------
 * Includes.
 */
#define __KERNEL__ 1

#include <sys/coherent.h>

#include <sys/cmn_err.h>
#include <sys/con.h>
#include <sys/devices.h>
#include <sys/errno.h>
#include <sys/io.h>
#include <sys/mmu.h>
#include <sys/poll.h>
#include <sys/seg.h>

#include <kernel/trace.h>

/* Tunable parameters. */

extern int	TRACE_BUF_LEN;

unsigned char	* traceBuf;

struct Trace {

	/* Booleans. */
	unsigned char	tr_open;

	/* Queue pointers and count. */
	unsigned int	tr_head;
	unsigned int	tr_tail;
	unsigned int	tr_count;
};

static struct Trace	tr;
static event_t		tr_ev;

/************************************************************************
 * trAlloc ()
 *
 * Attempt to acquire storage for the trace buffer.
 * In case of failure, leave traceBuf at its initial value of 0.
 ***********************************************************************/
#if	__USE_PROTO__
void trAlloc (void)
#else
void
trAlloc ()
#endif
{
	__caddr_t	devVaddr;
	int		devSeg;
	int		s;
	int		pagesNeeded;
	int		pte;
	int		i;
	__paddr_t	pageTablePaddr;
	__caddr_t	pageTableVaddr;
	int		* ptable;

_chirp ('U', 154);

	/* Expand paging services to allow for this buffer area. */

	pagesNeeded = btocru(TRACE_BUF_LEN);

	/*
	 * See if there are enough pages to do the allocation.
	 * Allow about 64 extra since the system will need some room to run.
	 */

	if (allocno () < pagesNeeded + 64) {
		return;
	}

_chirp ('V', 154);

	/* Compute the virtual address where device buffer memory starts. */

	devVaddr = DEVICE_SEG_VADDR(TRACE_MAJOR);

	/* Compute the segment number of the 4 MByte-aligned virtual segment. */

	devSeg = btosrd ((int)devVaddr);

	s = sphi ();

	/* Grab a page to map the buffer area for this device. */

	pageTablePaddr = (__paddr_t)(ctob (* -- sysmem.pfree));
	pageTableVaddr = __PTOV(pageTablePaddr);

	pte = (int)(pageTablePaddr) | SEG_RW;
	ptable0_v [devSeg] = pte;
	
_chirp ('W', 154);

	/* Grab and map the specified number of pages for the device buffer. */

	ptable = (int *)pageTableVaddr;

	for (i = 0; i < pagesNeeded; i++) {
		pte = ctob (* -- sysmem.pfree) | SEG_RW;
		* ptable ++ = pte;
	}

	spl (s);
	mmuupd ();

_chirp ('X', 154);

	traceBuf = (unsigned char *)devVaddr;

	cmn_err (CE_NOTE, "!trAlloc () - using %d pages at virt address %x",
	  pagesNeeded, (int)devVaddr);

_chirp ('Y', 154);

	return;
}

/************************************************************************
 * _put_putbuf ()
 *
 ***********************************************************************/
#if	__USE_PROTO__
void (_put_putbuf) (unsigned char outch)
#else
void
_put_putbuf __ARGS ((outch))
unsigned char	outch;
#endif
{
	int		s;

	/* If buffer didn't get allocated, do nothing. */
	if (traceBuf == NULL)
		return;

	s = sphi ();

	/* If queue is full, discard oldest contents. */
	while (tr.tr_count >= TRACE_BUF_LEN) {
		tr.tr_count --;
		tr.tr_tail ++;
		if (tr.tr_tail >= TRACE_BUF_LEN)
			tr.tr_tail = 0;
	}

	/* Assertion:  (tr.tr_count < TRACE_BUF_LEN). */

	traceBuf [tr.tr_head] = outch;
	tr.tr_count ++;
	tr.tr_head ++;
	if (tr.tr_head >= TRACE_BUF_LEN)
		tr.tr_head = 0;

	if (tr_ev.e_procp)
		pollwake (& tr_ev);

	spl (s);
}

/************************************************************************
 * tropen
 *
 ***********************************************************************/
#if	__USE_PROTO__
static void tropen(o_dev_t __NOTUSED(dev), int __NOTUSED(mode),
  int __NOTUSED(_flags), __cred_t * __NOTUSED(_credp))
#else
static void
tropen (dev, mode)
o_dev_t dev;
int mode;
#endif
{
	/* Only one open at a time. */

	if (tr.tr_open) {
		cmn_err (CE_WARN, "TR : Only one open at a time.");
		set_user_error (EBUSY);
		return;
	}

	if (traceBuf == NULL) {
		cmn_err (CE_WARN, "TR : Buffer not allocated.");
		set_user_error (EIO);
		return;
	}

	tr.tr_open = 1;
	return;
}

/************************************************************************
 * trclose
 *
 ***********************************************************************/
#if	__USE_PROTO__
static void trclose(o_dev_t __NOTUSED(dev),
  int __NOTUSED(_mode), int __NOTUSED(_flags), __cred_t * __NOTUSED(_credp))
#else
static void
trclose (dev)
o_dev_t dev;
#endif
{
	tr.tr_open = 0;
}

/************************************************************************
 * trread
 *
 ***********************************************************************/
#if	__USE_PROTO__
static void trread(o_dev_t __NOTUSED(dev), IO * iop,
  __cred_t * __NOTUSED(_credp))
#else
static void
trread(dev, iop)
o_dev_t dev;
IO * iop;
#endif
{
	int s;

	while (iop->io_ioc && tr.tr_count) {
		ioputc(traceBuf [tr.tr_tail], iop);
		s = sphi ();
		tr.tr_count --;
		spl (s);
		tr.tr_tail ++;
		if (tr.tr_tail >= TRACE_BUF_LEN)
			tr.tr_tail = 0;
	}
}

/************************************************************************
 * trpoll
 *
 ***********************************************************************/
#if	__USE_PROTO__
static int trpoll(o_dev_t __NOTUSED(dev), int ev, int msec)
#else
static int
trpoll(dev, ev, msec)
o_dev_t dev;
int ev;
int msec;
#endif
{
	/*
	 * Only POLLIN is supported.
	 */
	ev &= POLLIN;

	/*
	 * Input poll with no data present.
	 */
	if ((ev & POLLIN) != 0 && tr.tr_count == 0) {

		/*
		 * Blocking input poll.
		 */
		if (msec != 0)
			pollopen (& tr_ev);
	}

	return ev;
}

/*
 * Configuration table.
 */
CON trcon ={
	DFCHR | DFPOL,			/* Flags */
	TRACE_MAJOR,			/* Major index */
	tropen,				/* Open */
	trclose,			/* Close */
	NULL,				/* Block */
	trread,				/* Read */
	NULL,				/* Write */
	NULL,				/* Ioctl */
	NULL,				/* Powerfail */
	NULL,				/* Timeout */
	NULL,				/* Load */
	NULL,				/* Unload */
	trpoll				/* Poll */
};
