#

/* date routine for systems without the TOY clock */

#define	MONTH	itoa(tim[4]+1,cp,2)
#define	DAY	itoa(tim[3],cp,2)
#define	YEAR	itoa(tim[5],cp,2)
#define	HOUR	itoa(tim[2],cp,2)
#define	MINUTE	itoa(tim[1],cp,2)
#define	SECOND	itoa(tim[0],cp,2)
#define	JULIAN	itoa(tim[7]+1,cp,3)
#define	WEEKDAY	itoa(tim[6],cp,1)
#define	MODHOUR	itoa(h,cp,2)

char	month[12][3]	{
	"Jan","Feb","Mar","Apr",
	"May","Jun","Jul","Aug",
	"Sep","Oct","Nov","Dec"};

char	days[7][3]	{
	"Sun","Mon","Tue","Wed",
	"Thu","Fri","Sat"};

int	zero;
int	timbuf[2];
char	*tzname[2];
int	dmsize[];
char	cbuf[];
char	*cbp;

struct {
	char	name[8];
	char	tty;
	char	fill1;
	int	wtime[2];
	int	fill2;
} wtmp[2];

main(argc, argv)
	int	argc;
	int	**argv;
{
	int	tbuf[2];
	int	i;
	char	c;
	int	h;
	int	hflag;
	register	char	*cp;
	register	char	*aptr;
	int	*tim;
	char	buf[200];
	char	*tzn;
	extern int timezone, *localtime();
	int wf, tfailed;

	tfailed = 0;
	if(argc > 1) {
		cbp = argv[1];

		if(*cbp == '+')	{
			hflag = 0;
			for(cp=buf;cp< &buf[200];)*cp++ = '\0';
			time(tbuf);
			tim = localtime(tbuf);
			aptr = argv[1];
			aptr++;
			cp = buf;
			while(c = *aptr++) {
				if(c == '%')
					switch(*aptr++) {
						case '%':
							*cp++ = '%';
							continue;
						case 'n':
							*cp++ = '\n';
							continue;
						case 't':
							*cp++ = '\t';
							continue;
						case 'm' :
							cp = MONTH;
							continue;
						case 'd':
							cp = DAY;
							continue;
						case 'y' :
							cp = YEAR;
							continue;
						case 'D':
							cp = MONTH;
							*cp++ = '/';
							cp = DAY;
							*cp++ = '/';
							cp = YEAR;
							continue;
						case 'H':
							cp = HOUR;
							continue;
						case 'M':
							cp = MINUTE;
							continue;
						case 'S':
							cp = SECOND;
							continue;
						case 'T':
							cp = HOUR;
							*cp++ = ':';
							cp = MINUTE;
							*cp++ = ':';
							cp = SECOND;
							continue;
						case 'j':
							cp = JULIAN;
							continue;
						case 'w':
							cp = WEEKDAY;
							continue;
						case 'r':
							if((h = tim[2]) >= 12)	hflag++;
							if((h =% 12) == 0)  h = 12;
							cp = MODHOUR;
							*cp++ = ':';
							cp = MINUTE;
							*cp++ = ':';
							cp = SECOND;
							*cp++ = ' ';
							if(hflag) *cp++ = 'P';
							else *cp++ = 'A';
							*cp++ = 'M';
							continue;
						case 'h':
							for(i=0; i<3; i++)
								*cp++ = month[tim[4]][i];
							continue;
						case 'a':
							for(i=0; i<3; i++)
								*cp++ = days[tim[6]][i];
							continue;
						default:
							write(2,"date: invalid option\n",21);
							exit(1);
					}
				*cp++ = c;
			}

			*cp = '\n';
			write(1,buf,(cp - &buf[0]) + 1);
			exit(0);
		}

		if(*cbp == '-') {
			write(2,"date: no TOY clock\n",19);
			exit(1);
		}

		if(gtime()) {
			write(1, "bad conversion\n", 15);
			exit(1);
		}

	/* convert to Greenwich time, on assumption of Standard time. */
		dpadd(timbuf, timezone);

	/* Now fix up to local daylight time. */
		if (localtime(timbuf)[8])
			dpadd(timbuf, -1*60*60);

		time(wtmp[0].wtime);
		wtmp[0].tty =  '|';

		if(stime(timbuf) < 0) {
			tfailed++;
			write(1, "no permission\n", 14);
		} else if ((wf = open("/usr/adm/wtmp", 1)) >= 0) {
			time(wtmp[1].wtime);
			wtmp[1].tty = '}';
			seek(wf, 0, 2);
			write(wf, wtmp, 32);
		}

	}
	if (tfailed==0)
		time(timbuf);
	cbp = cbuf;
	ctime(timbuf);
	write(1, cbuf, 20);
	tzn = tzname[localtime(timbuf)[8]];
	if (tzn)
		write(1, tzn, 3);
	write(1, cbuf+19, 6);
}

gtime()
{
	register int i;
	register int y, t;
	int d, h, m;
	extern int *localtime();
	int nt[2];

	t = gpair();
	if(t<1 || t>12)
		goto bad;
	d = gpair();
	if(d<1 || d>31)
		goto bad;
	h = gpair();
	if(h == 24) {
		h = 0;
		d++;
	}
	m = gpair();
	if(m<0 || m>59)
		goto bad;
	y = gpair();
	if (y<0) {
		time(nt);
		y = localtime(nt)[5];
	}
	if (*cbp == 'p')
		h =+ 12;
	if (h<0 || h>23)
		goto bad;
	timbuf[0] = 0;
	timbuf[1] = 0;
	y =+ 1900;
	for(i=1970; i<y; i++)
		gdadd(dysize(i));
	/* Leap year */
	if (dysize(y)==366 && t >= 3)
		gdadd(1);
	while(--t)
		gdadd(dmsize[t-1]);
	gdadd(d-1);
	gmdadd(24, h);
	gmdadd(60, m);
	gmdadd(60, 0);
	return(0);

bad:
	return(1);
}

gdadd(n)
{
	register char *t;

	t = timbuf[1]+n;
	if(t < timbuf[1])
		timbuf[0]++;
	timbuf[1] = t;
}

gmdadd(m, n)
{
	register int t1;

	timbuf[0] =* m;
	t1 = timbuf[1];
	while(--m)
		gdadd(t1);
	gdadd(n);
}

gpair()
{
	register int c, d;
	register char *cp;

	cp = cbp;
	if(*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c<0 || c>100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	cbp = cp;
	return (c+d);
}

itoa(i,ptr,dig)
	register  int	i;
	register  int	dig;
	register  char	*ptr;
{
	switch(dig)	{
		case 3:
			*ptr++ = i/100 + '0';
			i = i - i / 100 * 100;
		case 2:
			*ptr++ = i / 10 + '0';
		case 1:	
			*ptr++ = i % 10 + '0';
	}
	return	(ptr);
}
