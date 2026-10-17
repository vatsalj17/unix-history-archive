# include	"../hdr/defines.h"

SCCSID(@(#)sinit	1.4);
/*
	Does initialization for sccs files and packet.
*/

sinit(pkt,file,openflag)
register struct packet *pkt;
register char *file;
{
	register char *p;

	zero(pkt,sizeof(*pkt));
	if (size(file) > FILESIZE)
		fatal("too long (co7)");
	if (!sccsfile(file))
		fatal("not an SCCS file (co1)");
	copy(file,pkt->p_file);
	pkt->p_wrttn = 1;
	if (openflag) {
		pkt->p_iop = xfopen(file,0);
		setbuf(pkt->p_iop,pkt->p_buf);
		fstat(fileno(pkt->p_iop),&Statbuf);
		if (Statbuf.i_nlink > 1)
			fatal("more than one link (co3)");
		if ((p = getline(pkt)) == NULL || *p++ != CTLCHAR || *p++ != HEAD) {
			fclose(pkt->p_iop);
			fmterr(pkt);
		}
		p = satoi(p,&pkt->p_ihash);
		if (*p != '\n')
			fmterr(pkt);
	}
	pkt->p_chash = 0;
}
