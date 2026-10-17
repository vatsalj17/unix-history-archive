# include	"../hdr/defines.h"

SCCSID(@(#)stats_ab	1.2);

stats_ab(pkt,statp)
register struct packet *pkt;
register struct stats *statp;
{
	register char *p;

	p = pkt->p_line;
	if (getline(pkt) == NULL || *p++ != CTLCHAR || *p++ != STATS)
		fmterr(pkt);
	NONBLANK(p);
	p = satoi(p,&statp->s_ins);
	p = satoi(++p,&statp->s_del);
	satoi(++p,&statp->s_unc);
}
