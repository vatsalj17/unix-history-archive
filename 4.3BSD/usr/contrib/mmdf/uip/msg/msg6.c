/*
 *			M S G 6 . C
 *
 * Functions -
 *	smtpdate	Converts RFC822 date string to UNIX long
 *	sunday
 *	eatwhite	Returns first nonwhitespace char in string
 *	match
 *	tzset
 *	makedate	Convert UNIX time to string
 *
 *
 *		R E V I S I O N   H I S T O R Y
 *
 *	07/08/84  HAW	Created this module for time stamp processing.
 *			Most routines in this module were written
 *			by Ron Natelie.
 *
 */
#include "util.h"
#include "mmdf.h"

extern	long	timezone;
extern	long	atol();
extern	int	daylight;
extern	char	*tzname[];
extern	char	*ctime();

/*
 *  S M T P _ D A T E
 *
 *  Take an RFC822 (more or less) date string cp and convert it to
 *  unix time.  Allows a variety of illegal formats commonly in use.
 *
 */

#define	SPACE	' '
#define	HTAB	'\t'

long	match();

/*
 *  Matchtab structure.
 *  Used by match routine.
 */
struct	match_tab  {
	char	*t_string;		/*  String to match, NULL for last.  */
	long	t_val;			/*  Value to be returned.  */
};

/*
 *  Month tab matches month names to number.
 */
struct	match_tab	month_tab[]  =  {
	"jan", 1,	"feb", 2,	"mar", 3,	"apr", 4,
	"may", 5,	"jun", 6,	"jul", 7,	"aug", 8,
	"sep", 9,	"oct", 10,	"nov", 11,	"dec", 12,
	NULL, 0
};
	
#define	H(x)	( (x)*60L*60L)
struct	match_tab	zone_tab[] = {
	"ut", 0,	"gmt", 0,	"est", H(-5),	"edt", H(-4),
	"cst", H(-6),	"cdt", H(-5),	"mst", H(-7),	"mdt", H(-6),
	"pst", H(-8),	"pdt", H(-7),	"met", H(1),	"z", 0,
	"a", H(-1),	"b", H(-2),	"c", H(-3),	"d", H(-4),
	"e", H(-5),	"f", H(-6),	"g", H(-7),	"h", H(-8),
	"i", H(-9),	"k", H(-10),	"l", H(-11),	"m", H(-12),
	"n", H(1),	"o", H(2),	"p", H(3),	"q", H(4),
	"r", H(5),	"s", H(6),	"t", H(7),	"u", H(8),
	"v", H(9),	"w", H(10),	"x", H(11),	"y", H(12),
	NULL, 1
};

#define	dysize(A) (((A)%4) ? 365: 366)
static int dmsize[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
extern char *index();
extern char *eatwhite();

long
smtpdate(cp)
	register char	*cp;
{
	int	day, month, year, hours, minutes, seconds;
	int	dayno;
	long	tval;
	int	i;
	char	*dp;
	time_t	zone;
/*printf("S entry - date str: %s", cp); */
	cp = eatwhite(cp);
	dp = cp;


	/*
	 *  Look for digit, supposed to be day of the month, skipping
	 *  over day of the week stuff.
	 */
	while(1)  {
		if(*cp == 0)  goto bad_date;
		if(isdigit(*cp)) break;
		cp++;
	}

	day = atoi(cp);
/*printf("S day: %d\n",day); */
	/*  Skip digits and white space to get to month.  */

	while(isdigit(*cp)) cp++;
	while( *cp == SPACE || *cp == HTAB || *cp == '-') cp++;

	month = (int)match(month_tab, cp, 3);
	if(month == 0)  {
		/*  Hack here, look for ctime format.  */
		day = atoi(&dp[8]);
		month = (int)match(month_tab, &dp[4], 3);
		if(month == 0) goto bad_date;
		year = atoi(&dp[20]);
		hours = atoi(&dp[11]);
		minutes = atoi(&dp[14]);
		seconds = atoi(&dp[17]);
		zone = -1;
/*printf("S ctime for: y: %d m: %d d: %d h: %d m: %d s: %d\n",year,month,day,hours,minutes,seconds); */

	}
	else  {
		/*  Skip over to the year  */
		while(1)  {
			if(*cp == 0)  goto bad_date;
			if(isdigit(*cp)) break;
			cp++;
		}
		year = atoi(cp);

		while(isdigit(*cp)) cp++;

		while(*cp && !isdigit(*cp)) cp++;
	

		/*  Hours, minutes, seconds  */
		hours = atoi(cp);
		if(hours > 60)  {
			minutes = hours %100;
			hours = hours/100;
		}  else  {
			cp = index(cp, ':');
			if(cp == NULL) goto bad_date;
			minutes = atoi(++cp);
		}

		dp = index(cp, ':');
		if(dp)  {
			seconds = atoi(++dp);
			cp = dp;
		}
		else	seconds = 0;
/*printf("S hours: %d  mins: %d  secs: %d\n",hours,minutes,seconds); */
		while(isdigit(*cp)) cp++;
		cp = eatwhite(cp);
/*printf("S Zone starts- str: %s",cp); */
		if( (*cp == '+' || *cp == '-')  &&
		   isdigit(cp[1]) &&
		   isdigit(cp[2]) &&
		   isdigit(cp[3]) &&
		   isdigit(cp[4])
		)  {
			zone = (time_t)(atol(cp+3) * 60L);
			cp[3] = 0;
			zone *= (time_t)atol(cp);
		} else {
			if( *cp == '+' || *cp == '-' )
				cp++;
			dp = cp;
			while(isalpha(*cp)) cp++;
			*cp = 0;
			zone = (time_t)(-match(zone_tab, dp, 0));
		}
/*printf("S zone: %d\n",zone); */
	}
	if(year < 1900) year += 1900;
	if(month < 1 || month > 12)  goto bad_date;
	if(day < 1 || day > 31) goto bad_date;
	if(hours == 24)  {
		hours = 0;
		day++;
	}
	if(hours < 0 || hours > 23) goto bad_date;
	if(minutes < 0 || minutes > 59) goto bad_date;
	if(seconds < 0 || seconds > 59) goto bad_date;

	tval = 0;
	dayno = 0;
	for(i = 1970; i < year; i++)
		tval += dysize(i);

	if(dysize(year) == 366  && month >= 3)
		dayno += 1;
	while(--month) dayno += dmsize[month-1];
	tval += dayno;
	tval += day-1;
	tval *= 24;
	tval += hours;
	tval *= 60;
	tval += minutes;
	tval *= 60;
	tval += seconds;
	if(zone == -1)  {
		int beg, end;
		beg = sunday(119, year);
		end = sunday(303, year);
/*printf("S beg: %d  end: %d\n",beg,end); */
		zone = timezone;		
/*printf("S timezone: %d  daylight: %d: dayno: %d\n",timezone,daylight,dayno); */
		if(daylight &&
		    (dayno>beg || (dayno==beg && hours >=2)) &&
		    (dayno<end || (dayno==end && hours <1)))
		{
/*printf("S changing zone for daylight\n"); */
			zone -= 60*60;
		}
	}
/*printf("S before zone: %s",ctime(&tval)); */
	tval += zone;
/*printf("S after zone: %s\n",ctime(&tval)); */
	return tval;

bad_date:
	return -1L;
}

static int
sunday(d, year)
{
	int i;
	int	pdays = 0;

	for(i = 1970; i < year; i++)
		pdays += dysize(i);
	if(d >= 58 && dysize(year) == 366) d++;
	pdays = (pdays - 3) % 7;	/*  Day of week, day 0 was thurs  */
	if(pdays) d += 7-pdays;
	return d;
}
/*
 *  E A T W H I T E
 *
 *  An old favorite, returns the first non whitespace in the string.
 */
char *
eatwhite(cp)
	register char *cp;
{
	while(*cp == SPACE || *cp == HTAB) cp++;
	return cp;
}

long
match(tab, string, size)
	register struct	match_tab	*tab;
	char	*string;
	int	size;
{
	register char *cp, *str;
	char	*bufend;
	char	buffer[512];
	
	bufend = &buffer[ (size == 0) ? 512 : size];
	for(cp = buffer, str = string; cp < bufend; cp++, str++)  {
		if(isupper(*str)) *cp = tolower(*str);
		else	*cp = *str;
		if(*cp == 0) break;
	}

	while(tab->t_string != NULL)  {
		if(size)  {
			if(strncmp(buffer,tab->t_string, size) == 0)
				break;
		}  else  {
			if(strcmp(buffer, tab->t_string) == 0)
				break;
		}
		tab++;
	}
	return tab->t_val;
}

#ifndef SYS5
void
tzset()
{
	register char *p, *q;
	register int n;
	int sign;
	char *getenv();
	if((p = getenv ("TZ")) && *p) {
		n = 3;
		q = tzname[0];
		do {
			*q++ = *p? *p++: ' ';
		} while(--n);
		if(sign = *p == '-')
			p++;
		n = 0;
		while(*p >= '0' && *p <= '9')
			n = (n * 10) + *p++ - '0';
		if(sign)
			n = -n;
		timezone = ((long)(n * 60)) * 60;
		if(daylight = *p != '\0') {
			q = tzname[1];
			n = 3;
			do {
				*q++ = *p? *p++: ' ';
			} while(--n);
		}
	}
	else	{			/* TZ not set in evironment */
#ifdef V4_2BSD
		extern int	gettimeofday();
#else
		extern int	ftime();
#endif
		static struct tzinfo	/* time zone names */
		{
			long	secwest;	/* sec west of GMT */
			char	*stdzone;	/* standard name */
			char	*dstzone;	/* DST name */
		}	tzi[] =
		{
			{	5L * 60L * 60L,	"EST",	"EDT"	},
			{	6L * 60L * 60L,	"CST",	"CDT"	},
			{	7L * 60L * 60L,	"MST",	"MDT"	},
			{	8L * 60L * 60L,	"PST",	"PDT"	},
			{	0L * 60L * 60L,	"GMT",	"?DT"	},
			{    (-1L) * 60L * 60L, "MET",  "?DT"   },
			/* add your favorite time zones here */
		};
		register struct tzinfo	*tzip;
#ifdef V4_2BSD
		struct {
			unsigned long	tv_sec;	/* since Jan. 1, 1970 */
			long		tv_usec;	/* microsec */
		}	tv;		/* real time from kernel */
		struct {
			long	tz_minuteswest;	/* of Greenwich */
			long	tz_dsttime;	/* "DST is in effect" */
		}	tz;		/* time zone data from kernel */
#else
		struct {
			time_t	time;
			unsigned short millitm;
			short	timezone;
			short	dstflag;
		}	tm;
#endif
#ifdef V4_2BSD
		if ( gettimeofday( &tv, &tz ) == 0 )
#else
		if ( ftime(&tm) == 0)
#endif
		{
#ifdef V4_2BSD
			timezone = tz.tz_minuteswest * 60L;
			daylight = tz.tz_dsttime != 0L;
#else
			timezone = tm.timezone * 60L;
			daylight = tm.dstflag != 0L;
#endif
			/* look zone names up in table */
			for ( tzip = tzi;
			      tzip < &tzi[sizeof tzi / sizeof tzi[0]];
			      ++tzip
			    )
				if ( tzip->secwest == timezone )
					{
					tzname[0] = tzip->stdzone;
					tzname[1] = tzip->dstzone;
					return;
					}
			/* zone names not found in table */
			tzname[0] = "?ST";
			tzname[1] = "?DT";
		}
		/* else use hard-coded default values */
	}
}
#endif SYS5

/*
 *			M A K E D A T E
 *
 * Convert UNIX date to string (DD MMM YY)
 */
makedate( date, dest )
long	*date;
char	*dest;
{
	char *cp;

	cp = ctime( date );
 	*dest++ = cp[8];
 	*dest++ = cp[9];
 	*dest++ = ' ';
 	*dest++ = cp[4];
 	*dest++ = cp[5];
 	*dest++ = cp[6];
 	*dest++ = ' ';
 	*dest++ = cp[22];
 	*dest++ = cp[23];

}
