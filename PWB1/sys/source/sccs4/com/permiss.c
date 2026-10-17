# include	"../hdr/defines.h"

SCCSID(@(#)permiss	1.4);

finduser(pkt)
register struct packet *pkt;
{
	register char *p;
	register int i;
	char *user;
	int none;

	none = 1;
	user = logname();
	while ((p = getline(pkt)) != NULL && *p != CTLCHAR) {
		none = 0;
		if (!pkt->p_user)
			for (i = 0; p[i]; i++) {
				if (p[i] == '\n') {
					if (!user[i])
						pkt->p_user = 1;
					break;
				}
				if (p[i] != user[i])
					break;
			}
	}
	if (none)
		pkt->p_user = 1;
	if (p == NULL || p[1] != EUSERNAM)
		fmterr(pkt);
}


char	*Sflags[NFLAGS];

doflags(pkt)
struct packet *pkt;
{
	register char *p;
	register int k;

	for (k = 0; k < NFLAGS; k++)
		Sflags[k] = 0;
	while ((p = getline(pkt)) != NULL && *p++ == CTLCHAR && *p++ == FLAG) {
		NONBLANK(p);
		k = *p++ - 'a';
		NONBLANK(p);
		Sflags[k] = alloc(size(p));
		copy(p,Sflags[k]);
		for (p = Sflags[k]; *p++ != '\n'; )
			;
		*--p = 0;
	}
}


permiss(pkt)
register struct packet *pkt;
{
	extern char *Sflags[];
	register char *p;
	register int n;

	if (!pkt->p_user)
		fatal("not authorized to make deltas (co14)");
	if (p = Sflags[FLORFLAG - 'a']) {
		if (pkt->p_reqsid.s_rel.unsignd < (n = patoi(p)))
			fatal(sprintf(Error,"release %u < %u (floor) (co15)",
				pkt->p_reqsid.s_rel,n));
	}
	if (p = Sflags[CEILFLAG - 'a'])
		if (pkt->p_reqsid.s_rel.unsignd > (n = patoi(p)))
			fatal(sprintf(Error,"release %u > %u (ceiling) (co16)",
				pkt->p_reqsid.s_rel,n));
}
