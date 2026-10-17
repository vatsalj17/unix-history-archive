

struct cblock {
	struct cblock *c_next;
#ifndef CBLOCK_16
	char info[6];
	};
	extern cblockm; extern cblockl;
#endif
#ifdef CBLOCK_16
	char info[14];
	};
	extern cblockm; extern cblockl;
#endif

/* The character lists-- space for (6 or 14)*NCLIST characters */
struct cblock cfree[NCLIST];
/* List head for unused character blocks. */
struct cblock *cfreelist;

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */
cinit()
{
	register int ccp;
	register struct cblock *cp;
	register struct cdevsw *cdp;

	ccp = cfree;
	for (cp=(ccp+cblockm)&~cblockm; cp <= &cfree[NCLIST-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
	}
	ccp = 0;
	for(cdp = cdevsw; cdp->d_open; cdp++)
		ccp++;
	nchrdev = ccp;
}
