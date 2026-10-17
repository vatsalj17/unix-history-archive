/* $Header: /ker/i386/RCS/work.c,v 2.4 93/10/29 00:58:17 nigel Exp Locker: nigel $ */
/*
 * Manage temporary pages of virtual memory.
 * Pages are allocated in virtually contiguous pairs to allow
 * for straddle operations in target code.
 *
 * $Log:	work.c,v $
 * Revision 2.4  93/10/29  00:58:17  nigel
 * R98 (aka 4.2 Beta) prior to removing System Global memory
 * 
 * Revision 2.3  93/08/19  03:40:20  nigel
 * Nigel's R83
 * 
 * Revision 2.2  93/07/26  13:57:36  nigel
 * Nigel's R80
 * 
 * Revision 1.1  93/04/14  10:29:40  root
 * r75
 */

#define	_KERNEL		1

#include <kernel/reg.h>
#include <sys/mmu.h>

#define	START_WORK	0xFFFFA		/* Highest scratchpad click pair. */
#define MAX_WORK_PAIRS	4		/* Max # in use at one time */

int	workAlloc();
void	workFree();
void	workPoolInit();

static int	numWorkPairs;	/* Number of click pairs in use. */

static int	workPool[MAX_WORK_PAIRS];

void
workPoolInit()
{
	int i, w;

	for (i = 0, w = START_WORK; i < MAX_WORK_PAIRS; i++, w -= 2)
		workPool[i] = w;
}

/*
 * Allocate a click pair of virtual space for temporary use.
 * Panic if none available.
 * Return value is a click number, e.g. 0xFFFFA, suitable for use as
 * an index into ptable1_v[].  The return value plus one is also
 * available for use as a virtual click number.
 */
int
workAlloc()
{
	int s, ret;

	s = sphi();
	if (numWorkPairs >= MAX_WORK_PAIRS)
		panic("Work pair pool empty");
	ret = workPool[numWorkPairs++];
	spl(s);
	return ret;
}

/*
 * Return a click pair of virtual space to the free pool.
 */
void
workFree(w)
int w;
{
	int s;

	if (w > START_WORK || w <= START_WORK - MAX_WORK_PAIRS)
		panic("workFree(%x)", w);

	ptable1_v [w + 1] = ptable1_v [w] = SEG_ILL;
	mmuupd ();

	s = sphi();
	if (numWorkPairs == 0)
		panic("Work pair pool exploded");
	workPool[--numWorkPairs] = w;
	spl(s);
}
