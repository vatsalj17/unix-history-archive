# include	"../hdr/defines.h"
# define DO2(p,n,c)	*p++ = n/10 + '0'; *p++ = n%10 + '0'; *p++ = c;

SCCSID(@(#)date_ba	1.2);

date_ba(bdt,adt)
long *bdt;
char *adt;
{
	register struct time *lcltm;
	register char *p;

	lcltm = localtime(bdt);
	p = adt;
	DO2(p,lcltm->t_year,'/');
	DO2(p,(lcltm->t_month + 1),'/');
	DO2(p,lcltm->t_day_month,' ');
	DO2(p,lcltm->t_hours,':');
	DO2(p,lcltm->t_minutes,':');
	DO2(p,lcltm->t_seconds,0);
	return(adt);
}
