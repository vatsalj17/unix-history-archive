/*
char id_rsfe[] = "@(#)rsfe.c	1.1";
 *
 * read sequential formatted external routines
 */

#include "fio.h"

int	x_getc(), x_rnew();
extern	int	rd_ed(), rd_ned(), x_tab();

char rsfe[] = "read sfe";

s_rsfe(a) cilist *a; /* start */
{	int n;
	reading = YES;
	if(n=c_sfe(a,READ)) return(n);
	if(curunit->uwrt && ! nowreading(curunit)) err(errflag, errno, rsfe)
	getn= x_getc;
	doed= rd_ed;
	doned= rd_ned;
	donewrec = dorevert = doend = x_rnew;
	dotab = x_tab;
	if(pars_f(fmtbuf)) err(errflag,F_ERFMT,rsfe)
	fmt_bg();
	return(OK);
}

x_rnew()			/* find next record */
{	int ch;
	if(curunit->uend)
		return(EOF);
	while((ch=getc(cf))!='\n' && ch!=EOF);
	if(feof(cf))
	{	curunit->uend = YES;
		if (recpos==0) return(EOF);
	}
	cursor=recpos=reclen=0;
	return(OK);
}

x_getc()
{	int ch;
	if(curunit->uend) return(EOF);
	if((ch=getc(cf))!=EOF && ch!='\n')
	{	recpos++;
		return(ch);
	}
	if(ch=='\n')
	{	ungetc(ch,cf);
		return(ch);
	}
	if(feof(cf)) curunit->uend = YES;
	return(EOF);
}
