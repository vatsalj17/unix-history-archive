#

/*
 * Selector channel scheduling routines
 */

#include "param.h"
#include "buf.h"
#include "selch.h"

#define	NSELCH	2	/* number of channels */

char	selchaddr[NSELCH] = {
	0xf0,
	0xf1
};

#define devmap(addr)	(addr&03)	/* map channel address=>channel no. */

/*
 * Queue of devices using selch
 *	- top of queue is currently active device; others are waiting
 */
struct {
	struct selchq	*sc_actf;
	struct selchq	*sc_actl;
} selchtab[NSELCH];

/*
 * Request use of selch #n
 */
selchreq(n, selchq)
struct selchq *selchq;
{
	register struct selchq *s;
	register struct selchtab *st;
	register free;
	trace(040<<16, "schrequest", selchq);

	st = &selchtab[n];
	/* Make sure request not already on queue */
	for (s = st->sc_actf; s; s = s->sq_forw)
		if (s == selchq)
			return;

	(s = selchq)->sq_forw = 0;
	if (st->sc_actf) {
		st->sc_actl->sq_forw = s;
		free  = 0;
	} else {
		st->sc_actf = s;
		free = 1;
	}
	st->sc_actl = s;

	if (free)
		(*s->sq_sstart)();
}

/*
 * Free selch #n for next device
 */
selchfree(n)
{
	register struct selchq *s;
	register struct selchtab *st;

	st = &selchtab[n];
	trace(040<<16, "schfree", st->sc_actf);
	if ((s = st->sc_actf) == 0)
		return;
	st->sc_actf = s = s->sq_forw;

	if (s)
		(*s->sq_sstart)();
}

/*
 * Selector channel interrupt -- handled by currently active device
 */
selchintr(dev, stat)
{
	register struct selchq *s;
	register struct selchtab *st;
	trace(040<<16, "interrupt", dev);
	trace(040<<16, "status", stat);

	st = &selchtab[devmap(dev)];
	if (s = st->sc_actf)
		(*s->sq_sintr)(dev, stat);
}
