/*
 *
 *	Can be run in two ways (depending on its name(argv[0]))
 *
 *	alarm - send a message to the terminal at the specified time
 *
 *	runat - execc the given command and arguments at the specified time
 *
 *	Times are specified in the form: Fri Jun 1 9:53:14 1979
 *	Any of the arguments (except for the hours:minutes) may be ommited,
 *	the current date, month, year, being assumed.
 *	The arguments may be specified in any order.
 *	Alarm (Runat) also recognises "am", "pm", and "tomorrow" as valid
 *	arguments.
 *
 *					Michael Rourke - June 1979
 */

#include <stdio.h>
#include <date.h>

#define MAXSLEEP 65535l
#define SECINDAY 86400l
#define SECINHOUR 3600l
#define SECINMIN 60l

int daysinmonth[]
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
char *dayandmonth[]
{
	"jan", "feb", "mar", "apr", "may", "jun",
	"jul", "aug", "sep", "oct", "nov", "dec",
	"sun", "mon", "tue", "wed", "thu", "fri", "sat",
	"tom", "am", "pm"
};
int hour, minute, second, day, year, dayofweek, month, nargs, relative;
char sdayofweek, syear, smonth, sday, stime, runat, srelative, spm, pm;
long mtime(), time(), systime;
struct date *localtime(), *tim;

main(argc, argv)
register int argc;
register char *argv[];
{
	long ttime;

	runat = (argv[0][0] == 'r');
	systime = time();
	tim = localtime(systime);
	day = tim->d_dyofmon;
	year = tim->d_year + 1900;
	month = tim->d_month;
	if((argc == 1) || getargs(argc, argv) || (runat && ((argc - nargs - 1) == 0)) || ( ! runat && (nargs != argc)))
		if(runat)
		{
			fprintf(stderr, "Usage:\nrunat [day] [month] [date] hours:minutes[:seconds] [year] : command\n");
			fprintf(stderr, "runat +minutes : command\n");
		}
		else
		{
			fprintf(stderr, "Usage:\nalarm [day] [month] [date] hours:minutes[:seconds] [year]\n");
			fprintf(stderr, "alarm +minutes\n");
		}
	else
	{
		if((ttime = mtime() - systime) <= 0)
		{
			fprintf(stderr, "This time has already passed\n");
			exit(-1);
		}
		go(ttime, argc - nargs - 1, &argv[nargs+1]);
	}
}

getargs(argc, argv)
int argc;
register char *argv[];
{
	int thour, tmin, tsec, tint, k, ch,  found;
	register int j, arg;
	char tstr[3];

	for(arg = 1; (arg < argc) && (argv[arg][0] != ':'); arg++)
	{
		thour = tmin = tsec = tint = 0;
		if(argv[arg][0] >= 'A')		/* a string */
		{
			for(j = 0; argv[arg][j] && j < 3; j++)	/* map upper to lower case */
				if((argv[arg][j] >= 'A') && (argv[arg][j] <= 'Z'))
					tstr[j] = argv[arg][j] + 'a' - 'A';
				else
					tstr[j] = argv[arg][j];
			found = 0;
			for(j = 0; (j < 12 + 7 + 3) && (! found); j++)	/* look for string */
				for(k = 0; k < 3; k++)
					if(tstr[k] != dayandmonth[j][k])
						break;
					else if( ! dayandmonth[j][k + 1])
						found++;
			if(found)
				if(--j < 12)	/* a month */
				{
					smonth++;
					month = j;
				}
				else if(j < 12 + 7)	/* a day of week */
				{
					sdayofweek++;
					dayofweek = j - 12;
				}
				else switch(j)
				{
					case 12 + 7:
						/* tomorrow */
						sdayofweek++;
						dayofweek = (tim->d_dyofwk + 1) % 7;
						break;
					case 12 + 7 + 1:
						/* am */
						spm++;
						break;
					case 12 + 7 + 2:
						/* pm */
						spm++;
						pm++;
						break;
				}
			else
			{
				fprintf(stderr, "Unidentified argument: \"%s\"\n", argv[arg]);
				return(1);
			}
		} 
		else if(sscanf(argv[arg], "%d:%d:%d", &thour, &tmin, &tsec) > 1)	/* the time */
		{
			if((thour >= 0) && (thour < 24) && (tmin >= 0) && (tmin < 60) && (tsec >= 0) && (tsec < 60))
			{
				stime++;
				hour = thour;
				minute = tmin;
				second = tsec;
			}
			else
			{
				fprintf(stderr, "Time out of range\n");
				return(1);
			}
		}
		else if(sscanf(argv[arg], "+%d%c", &tint, &ch) == 1)	/* a relative time */
		{
			srelative++;
			relative = tint * SECINMIN;
		}
		else if(sscanf(argv[arg], "%d%c", &tint, &ch) == 1)	/* a date or year */
			if(tint > 31)	/* year */
			{
				syear++;
				year = tint;
			}
			else if((tint > 0) && (tint <= 31))	/* day */
			{
				sday++;
				day = tint;
			}
			else
			{
				fprintf(stderr, "Date out of range\n");
				return(1);
			}
		else
		{
			fprintf(stderr, "Unidentified argument: \"%s\"\n", argv[arg]);
			return(1);
		}
		if((sday > 1) || (syear > 1) || (smonth > 1) || (sdayofweek > 1) || (stime > 1) || (srelative > 1) || (spm > 1))
		{
			fprintf(stderr, "Repeated argument: \"%s\"\n", argv[arg]);
			return(1);
		}
	}
	nargs = arg;
	if(srelative)
		return((arg != 2) || (runat && (argv[arg][0] != ':')));
	else
		return(( ! stime) || (runat && (argv[arg][0] != ':')));
}

long mtime()
{
	register int i, disp, days 0;

	if(srelative)
		return(systime + relative);
	for(i = 1970; i < year; i++)
		if(i % 4)
			days =+ 365l;
		else
			days =+ 366l;	/* a leap year */
	if(year % 4 == 0)
		daysinmonth[1] = 29;
	for(i = 0; i < month; i++)
		days =+ daysinmonth[i];
	days =+ day - 1;		/* days since 1 Jan 1970 */
	if(sdayofweek && ! sday) {
		disp = dayofweek - tim->d_dyofwk;
		if(disp < 0)
			disp =+ 7;
		days =+ disp;
	}
	if(pm)
		hour =+ 12;
	return(days * SECINDAY + hour * SECINHOUR + minute * SECINMIN + second);
}

go(wait, argc, argv)
long wait;
int argc;
char *argv[];
{
	register unsigned shortw, longw, i;

	switch(fork()) {
		case -1:
			fprintf(stderr, "Cannot fork\n");
			exit(-1);
		case 0:
			/* child */
			break;
		default:
			/* parent */
			fprintf(stderr, "It is now %s", ctime(systime));
			if(runat)
				fprintf(stderr, "Runat is set for %s",ctime(systime+wait));
			else
				fprintf(stderr, "Alarm is set for %s",ctime(systime+wait));
			exit(0);
	}
	signal(2, 1);
	signal(3, 1);
	longw = wait / MAXSLEEP;
	shortw = wait % MAXSLEEP;
	for(i = 0; i < longw; i++)
		sleep(MAXSLEEP);
	sleep(shortw);
	if(runat)
	{
		argv[argc] = 0;
		execc(*argv, argv);
		fprintf(stderr, "\nrunat: cannot execc \"%s\"\n", *argv);
	}
	else
		fprintf(stderr, "\07\nWarning, the time is %s\07",ijtime());
	exit(0);
}
