/*
char id_riio[]= "@(#)riio.c	1.1";
 *
 * read internal (character array) i/o
 */

#include "fio.h"
#include "lio.h"

int	z_rnew();
extern	l_read();
extern	int	rd_ed(), rd_ned(), z_tab();

z_getc()
{
	if(icptr >= icend && !recpos)	/* new rec beyond eof */
	{	leof = EOF;
		return(EOF);
	}
	if(recpos++ < svic->icirlen) return(*icptr++);
	return(' ');
}

z_ungetc(ch,cf) char ch;
{	if(ch==EOF || --recpos >= svic->icirlen) return(OK);
	if(--icptr < svic->iciunit || recpos < 0) err(errflag,F_ERBREC,"ilio")
	*icptr = ch;
	return(OK);
}

s_rsfi(a) icilist *a;
{
	reading = YES;
	doed=rd_ed;
	doned=rd_ned;
	getn=z_getc;
	doend = donewrec = z_rnew;
	dorevert = z_rnew;
	dotab = z_tab;
	return(c_si(a));
}

s_rdfi(a) icilist *a;
{
	reading = YES;
	doed = rd_ed;
	doned = rd_ned;
	getn = z_getc;
	donewrec = z_rnew;
	dorevert = doend = z_rnew;
	dotab = z_tab;
	return(c_di(a));
}

z_rnew()
{
	icptr = svic->iciunit + (++recnum)*svic->icirlen;
	recpos = reclen = cursor = 0;
	return(OK);
}

e_rsfi()
{	int n;
	n = en_fio();
	fmtbuf = NULL;
	return(n);
}

e_rdfi()
{
	return(e_rsfi());
}

s_rsli(a) icilist *a;
{
	reading = YES;
	lioproc = l_read;
	getn = z_getc;
	ungetn = z_ungetc;
	l_first = YES;
	lcount = 0;
	lquit = NO;
	return(c_li(a));
}

e_rsli()
{	fmtbuf = NULL;
	return(OK);
}
