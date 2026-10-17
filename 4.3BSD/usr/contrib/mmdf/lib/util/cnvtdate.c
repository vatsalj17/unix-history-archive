#include "util.h"
#include "conf.h"
#include "cnvtdate.h"

#ifndef SYS5
#include <sys/timeb.h>
#endif SYS5

#ifdef SYS5
#define timezone envtimezone
#endif SYS5

#ifdef ALTOS
#define timezone mytimezone
#endif ALTOS


char *
cnvtdate (flag, datbuf)   /* produce a date/time  string        */
int     flag;                     /* date format option value           */
char   *datbuf;
{
	static char *day[] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static char *month[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
#ifndef SYS5
	struct timeb    timeb;
#else SYS5
	time_t tsec;
#endif SYS5
	extern char *timezone();
	extern struct tm  *localtime ();
	register struct tm  *i;

#ifndef SYS5
	ftime (&timeb);
	i = localtime ((time_t *) & timeb.time);
#else SYS5
	time (&tsec);
	i = localtime (&tsec);
#endif SYS5

	switch (flag) {
	case TIMCOM:
		sprintf (datbuf, "%02d%02d%02d%02d%02d",
			i -> tm_year, i -> tm_mon + 1, i -> tm_mday,
			i -> tm_hour, i -> tm_min);
		break;

	case TIMSECS:             /* w/seconds dd mon yy hh:mm:ss       */
		sprintf (datbuf, "%d %s %d %d:%02d:%02d",
			i -> tm_mday, month[i -> tm_mon], i -> tm_year,
			i -> tm_hour, i -> tm_min, i -> tm_sec);
		break;

	case TIMREG:              /* RFC 822 standard time string       */
	default:                  /* "Wed, 21 Jan 76 14:30 PDT"        */
		sprintf (datbuf, "%s, %d %s %d %d:%02d:%02d %s",
			day[i -> tm_wday], i -> tm_mday, month[i -> tm_mon],
			i -> tm_year, i -> tm_hour, i -> tm_min, i -> tm_sec,
#ifndef SYS5
			timezone (timeb.timezone, i -> tm_isdst));
#else
			timezone (0, i -> tm_isdst));
#endif SYS5
		break;

	case TIMSHRT:             /* w/out day of week                  */
		sprintf (datbuf, "%d %s %d %d:%02d %s",
			i -> tm_mday, month[i -> tm_mon], i -> tm_year,
			i -> tm_hour, i -> tm_min,
#ifndef SYS5
			timezone (timeb.timezone, i -> tm_isdst));
#else
			timezone (0, i -> tm_isdst));
#endif SYS5
		break;
	}
	return(datbuf);
}

#ifdef SYS5
char *
envtimezone( ignored, dst )
int ignored;
int dst;
{
	extern char *tzname[2];
	static int doneit = 0;
	static char lcltz[8] = "???";
	extern char *strncpy();
	char *cp;

	if (!doneit) {
		doneit++;
		tzset();
	}
	cp = tzname[dst ? 1 : 0];
	if (cp == (char *)0 || *cp == '\0')
		cp = "???";
	(void) strncpy(lcltz, cp, sizeof(lcltz) - 1);
	return(lcltz);
}
#endif SYS5

#ifdef	ALTOS		/* sigh */
#define	abs(a)	((a) >= 0 ? (a) : -(a))

struct keywd {
	char   *key;
	int     value;
};

#define	NZONES	(sizeof (zones)/sizeof (struct keywd))

static struct keywd zones[] = {	/* order sensitive */
	"UT", 0,
	"EST", -5, "CST", -6,  "MST", -7, "PST", -8, "PDT", -9,
	"A",   -1, "B",   -2,  "C",   -3, "D",   -4,
	"E",   -5, "F",   -6,  "G",   -7, "H",   -8,
	"I",   -9, "K",  -10,  "L",  -11, "M",  -12,
	"N",    1, "O",    2,  "P",    3, "Q",    4,
	"R",    5, "S",    6,  "T",    7, "U",    8,
	"V",    9, "W",   10,  "X",   11, "Y",   12
};

char   *mytimezone (zone, dst)
int     zone, dst;
{
	int     i,
	j,
	hours,
	mins;
	static char buffer[10];

	zone = 0 - zone;

	if (zone < 0) {
		mins = -((-zone) % 60);
		hours = -((-zone) / 60);
	} else {
		mins = zone % 60;
		hours = zone / 60;
	}

	if (dst)
		hours -= 1;

	if (mins == 0)
		for (i = 0, j = NZONES; i < j; i++)
			if (zones[i].value == hours) {
				(void) strcpy (buffer, zones[i].key);
				if (dst && buffer[1] == 'S')
					buffer[1] = 'D';
				return buffer;
			}

	sprintf (buffer, "%s%02d%02d",
		zone < 0 ? "-" : "+", abs (hours), abs (mins));
	return buffer;
}
#endif ALTOS
