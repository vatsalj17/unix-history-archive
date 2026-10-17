/*
char id_wiio[] = "@(#)wiio.c	1.1";
 *
 * write internal (character array) i/o
 */

#include "fio.h"
#include "lio.h"

int	z_wnew(), z_tab();
extern	int	w_ed(), w_ned(), l_write();

z_putc(c) char c;
{
	if(icptr < icend)
	{	if(c=='\n') return(z_wnew());
		if(recpos++ < svic->icirlen)
		{	*icptr++ = c;
			return(OK);
		}
		else err(errflag,F_EREREC,"iio")
	}
	leof = EOF;
#ifndef KOSHER
	err(endflag,EOF,"iio")   /* NOT STANDARD, end-of-file on writes */
#endif
#ifdef KOSHER
	err(errflag,F_EREREC,"iio")
#endif
}

s_wsfi(a) icilist *a;
{
	reading = NO;
	doed=w_ed;
	doned=w_ned;
	putn=z_putc;
	doend = donewrec = z_wnew;
	dorevert = z_wnew;
	dotab = z_tab;
	return(c_si(a));
}

s_wdfi(a) icilist *a;
{
	reading = NO;
	doed = w_ed;
	doned = w_ned;
	putn = z_putc;
	donewrec = z_wnew;
	dorevert = doend = z_wnew;
	dotab = z_tab;
	return(c_di(a));
}

z_wnew()
{
	if(reclen > recpos)
	{	icptr += (reclen - recpos);
		recpos = reclen;
	}
	while(recpos < svic->icirlen) (*putn)(' ');
	recpos = reclen = cursor = 0;
	recnum++;
	return(OK);
}

e_wsfi()
{
	return(e_rsfi());
}

e_wdfi()
{
	return(e_wsfi());
}

s_wsli(a) icilist *a;
{
	reading = NO;
	putn = z_putc;
	lioproc = l_write;
	line_len = a->icirlen;
	return(c_li(a));
}

e_wsli()
{	fmtbuf = NULL;
	reclen = recpos;
	return(z_wnew());
}
