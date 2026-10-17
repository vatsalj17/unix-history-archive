# include	"../hdr/defines.h"

SCCSID(@(#)fmterr	1.2);

fmterr(pkt)
register struct packet *pkt;
{
	fclose(pkt->p_iop);
	fatal(sprintf(Error,"format error at line %u (co4)",pkt->p_slnno));
}
