#include "remind.h"

dtime()
/* Compute delivery time.  Return 1 if NOW else 0. */
{
	register struct tm *p;
	register int f;
	char datespec; /* Date specified flag */

	m.tsent = time((time_t *)0);
#ifdef DEBUG
	fprintf(stderr,"tsent = %D\n",m.tsent);
#endif
	p = localtime(&m.tsent);
	/* If some other day and no time spec, make 12:00 am. */
	if ( (datespec = (month|day|year) != 0) && (hour|minute) == 0)
		hour = minute = ABSOLUTE;

	/* Compute delivery time */

	f = fix(minute, p->tm_min);
	p->tm_hour += f/60;
	minute = f%60;

	f = fix(hour, p->tm_hour);
	p->tm_mday += f/24;
	hour = f%24;

	day = fix(day, p->tm_mday);
	while (day > (f = (p->tm_mon==1 && p->tm_year%4 ? 29 : dmsize[p->tm_mon])))
	{	day -= f;
		if (++p->tm_mon >= 12)
		{	p->tm_mon = 0;
			++p->tm_year;
		}
	}

	if (month&ABSOLUTE)
		month = (f = month&~ABSOLUTE) ? --f : f;
	else
		month += p->tm_mon;
	p->tm_year += month/12;
	month %= 12;
	++month;

	year = fix(year, p->tm_year);
	if (year < 10) year += 70;

	/* Convert to seconds past 0000 1 Jan 1970 GMT using
	 * algorithm in date.c
	 */
	gtime();

#ifdef DEBUG
	fprintf(stderr,"tdeliver is %D\ttsent is %D\n",m.tdeliver,m.tsent);
	fprintf(stderr,"sent: %s",ctime(&m.tsent));
	fprintf(stderr,"deliver: %s",ctime(&m.tdeliver));
#endif DEBUG
	if ((f = tdiff(&m.tdeliver,&m.tsent)) > 0)
		return(0);
	else if (datespec == 0  && f < -60)  /* Try tomorrow, same time */
	{       for (f = 3; f--; )  /* Add 8 hours 3 times */
			m.tdeliver += 28800;
		if (tdiff(&m.tdeliver,&m.tsent) > 0)
			return(0);
	}
	return(1);
}

fix(x,y)
int x,y;
{	return(x&ABSOLUTE ? x&~ABSOLUTE : y + x);
}

tdiff(tim1,tim2)
struct dp_int *tim1, *tim2;
{	/* Returns min(tim1-tim2, sgn(tim1-tim2)*32767) */
	register char *i, *j;
	register int k;
	if (tim1->unsgnd1 < tim2->unsgnd1)
		return (-32767);
	if (tim1->unsgnd1 > tim2->unsgnd1)
		return (32767);
	if ((i = tim1->unsgnd2) >= (j = tim2->unsgnd2))
	{       k = i - j;
		return (k&0100000 ? 32767 : k);
	}
	else
	{       k = j - i;
		return (k&0100000 ? -32767 : -k);
	}
}

/* Get date or time. */
getdt(pp)
char *pp;
{	char *p;
	register int i;
	p = pp;
	switch (*p++)
	{
	  case ':':
		minute = getinteg(&p);
		return(*p);
	  case '/':
		return(getdayr(&p));
	}
	p--;
	i = getinteg(&p);
	if (p==pp) return(1);
	switch (*p++)
	{
	  case '/':
		month = i;
		return(getdayr(&p));
	  case 'a':
	  case 'p':
	  case '\0':
		p--;
		hour = i;
		minute = hour&ABSOLUTE; /* If hr is relative, */
		break;			/* so is assumed mins */
	  case ':':
		hour = i;
		minute = getinteg(&p);
		if ((hour&ABSOLUTE) == 0 && (minute&ABSOLUTE))
			minute &= ~ABSOLUTE;
	}
	if (hour&ABSOLUTE)
		if (*p == 'p')
		{	if (p[1] == 'm')
			{	if ((hour&~ABSOLUTE) < 12)
					hour += 12;
				p += 2;
			}
		}
		else if (*p == 'a')
		{	if (p[1] == 'm')
			{	if ((hour&~ABSOLUTE) == 12)
					hour -= 12;
				p += 2;
			}
		}
	return(*p);
}

getdayr(pp)
char **pp;
{

	day = getinteg(pp);
	if (*(*pp) == '/')
	{	(*pp)++;
		year = getinteg(pp);
	}
	return (*(*pp));
}

getinteg(pp)
char **pp;
{	register char *p;
	register int i;
	register int d;
	int flag;

	p = *pp;
	i = 0;
	if (*p == '+')
	{	flag = 0;
		p++;
	}
	else flag = ABSOLUTE;
	while ((d=digit(*p++)) >= 0)
	{	i *= 10;
		i += d;
	}
	if (--p == *pp) flag = 0; /* Treat null field as incremental 0 */
	*pp = p;
	return (flag|i);
}

digit(c)
char c;
{	return (c>='0' && c<='9' ? c - '0' : -1);
}
gtime()
{
	register int i;
	register struct tm *tp;
	int tzone;
	struct timeb T;

	ftime(&T);
	tzone = T.timezone*60;
	m.tdeliver = 0L;
	year += 1900;
	for(i=1970; i<year; i++)
		gdadd(dysize(i));
	gdadd((year%4==0 && month>2 ? day : day-1));
	while(--month)
		gdadd(dmsize[month-1]);
	gmdadd(24, hour);
	gmdadd(60, minute);
	gmdadd(60, 0);
	/* convert to Greenwich time, on assumption of Standard time. */
	
		m.tdeliver += tzone;
	/* Now fix up to local daylight time. */
		tp = localtime(&m.tdeliver);
		if (tp->tm_isdst)
			m.tdeliver -= 60*60;
	return(0);
}

gmdadd(k, n)
{	/* 0<m<127 (maybe more, but 127 is safe) */
	register int sc;  /* sum/carry */
	register char *c;
	c = (char *)&m.tdeliver;
	c[2] = sc = k * (c[2]&0377);
	sc = sc>>8 & 0377;
	c[3] = sc += k * (c[3]&0377);
	sc = sc>>8 & 0377;
	c[0] = sc += k * (c[0]&0377);
	sc = sc>>8 & 0377;
	c[1] = sc + k * (c[1]&0377);
	gdadd(n);
}
