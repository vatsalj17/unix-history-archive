#
#include "param.h"
#include "tty.h"
#include "systm.h"

extern struct cblock *cfreelist;

#define	CBS	(sizeof *cfreelist)

/*
 * character-list manipulation routines
 */

/*
 * getc:
 *	return the next character from the given clist.
 *	if the clist is empty, return -1.
 */

getc(clist)
struct clist *clist;
{
	register struct clist *cl;
	register char c, *p;
	int sps;

	cl = clist;
	sps = spl(5);

	if ((p = cl->c_cf) == NULL) {
		cl->c_cl = NULL;
		spl(sps);
		return(-1);
	}
	c = *p;
	cl->c_cf = ++p;
	if (--cl->c_cc <= 0) {
		cl->c_cf = cl->c_cl = NULL;
		p = --p & ~(CBS -1);
		p->c_next = cfreelist;
		cfreelist = p;
	}
	else if ((p & (CBS -1)) == 0) {
		p =- CBS;
		cl->c_cf = p->c_next;
		p->c_next = cfreelist;
		cfreelist = p;
	}

	spl(sps);
	return(c);
}

/*
 * putc:
 *	put a character onto the given clist.
 *	if there are no more cblocks available, return nonzero
 *	  otherwise return 0.
 */

putc(ch, clist)
struct clist *clist;
{
	register struct clist *cl;
	register char c, *p, *q;
	int sps;

	cl = clist;
	c = ch;
	sps = spl(5);

	if ((p = cl->c_cl) == NULL) {
		if ((p = cfreelist) == NULL) {
			spl(sps);
			return(1);
		}
		cfreelist = p->c_next;
		p->c_next = NULL;
		cl->c_cc = 0;
		p =+ sizeof p->c_next;
		cl->c_cf = p;
	}
	else if ((p & (CBS -1)) == 0) {
		q = p-CBS;
		if ((p = cfreelist) == NULL) {
			spl(sps);
			return(1);
		}
		cfreelist = p->c_next;
		p->c_next = NULL;
		p =+ sizeof p->c_next;
		q->c_next = p;
	}
	*p++ = c;
	cl->c_cl = p;
	++cl->c_cc;

	spl(sps);
	return(0);
}
