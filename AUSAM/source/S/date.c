#
/*
 *	date [[[[[yy]mm]dd]hh]mm]
 *
 *	Piers Lauder	Mar '78
 */ 

#include	<local-system>

#ifdef	AUSAM

long time();
long timbuf;
int dmsize[];
char *ijtime();
char *cbp;
char c;

#include	<wtmp.h>
struct dtmp dtmp;



main(argc, argv)
int argc;
char **argv;
{
	register int wf;


	if(argc > 1)
	{
		cbp = argv[1];
		timbuf = time();
		dtmp.d_oldtime = timbuf;
		dtmp.d_type = D_TYPE;

		if(gtime())
		{
			write(1, "Bad conversion\n", 15);
			return 1;
		}

		if( peopleon() )
		{
			prints(1, "Too many people logged on\n");
			return 1;
		}
		if(timbuf < dtmp.d_oldtime || timbuf > dtmp.d_oldtime+3600)
		{
			if(timbuf < dtmp.d_oldtime)
				prints(1, "The date you have given is prior to the current time");
			else
				prints(1, "The date given is more than one hour in advance");
			prints(1, " confirm this change (y or n) : ");
			read(0, &c, 1);
			wf = c;
			while(c != '\n' && read(0, &c, 1) == 1);
			if(wf != 'y')
				return 1;
		}
		if(stime(timbuf) < 0)
		{
			write(1, "No permission\n", 14);
			return 1;
		}
		if((wf = open("/usr/adm/wtmp", 1)) >= 0)
		{
			dtmp.d_newtime = time();
			seek(wf, 0, 2);
			write(wf, &dtmp, sizeof dtmp);
		}
	}

	cbp = ijtime();
	write(1, cbp, 20);
	write(1, "Sydney Time", 11);
	write(1, cbp+19, 6);
	return 0;
}

gtime()
{
	register int i;
	register int y;
	register int *tvec;
	int t, d, h, m;
	extern int *localtime();


	tvec = localtime(timbuf);

	if((y = gpair()) < 0)
		goto bad;
	if((t = gpair()) < 0)
		goto setmin;
	if((d = gpair()) < 0)
		goto sethrs;
	if((h = gpair()) < 0)
		goto setday;
	if((m = gpair()) < 0)
		goto setmon;

setime:
	if(t < 1 || t > 12)
		goto bad;
	if(d < 1 || d > 31)
		goto bad;
	if(h >= 24)
	{
		if(h > 24)
			goto bad;
		h = 0;
		if(++d > 31)
		{
			d = 1;
			if(++t > 12)
			{
				t = 1;
				y++;
			}
		}
	}
	if(m > 59)
		goto bad;

	timbuf = 0;
	y =+ 1900;
	for(i = 1970; i < y; i++)
		timbuf =+ dysize(i);
	/* Leap year */ 
	if(dysize(y) == 366 && t >= 3)
		timbuf++;
	while(--t)
		timbuf =+ dmsize[t-1];
	timbuf =+ d-1;
	timbuf = timbuf*24+h;
	timbuf = timbuf*60+m;
	timbuf =* 60;
	return(0);

setmin:
	m = y;
	h = tvec[2];
getday:
	d = tvec[3];
getmon:
	t = tvec[4]+1;
getyrs:
	y = tvec[5];
	goto setime;

sethrs:
	m = t;
	h = y;
	goto getday;

setday:
	m = d;
	h = t;
	d = y;
	goto getmon;

setmon:
	m = h;
	h = d;
	d = t;
	t = y;
	goto getyrs;

bad:
	return(1);
}

gpair()
{
	register int c, d;
	register char *cp;


	cp = cbp;
	if(*cp == 0)
		return(-1);
	c = (*cp++ -'0')*10;
	if(c < 0 || c > 100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if((d = *cp++ -'0') < 0 || d > 9)
		return(-1);
	cbp = cp;
	return(c+d);
}


/*
 *	this routine checks if people other than root are logged on.
 *	and returns 1 if so.
 */
peopleon()
{
	register ufd;
	struct utmp u;

	if((ufd = open("/etc/utmp", 0)) == -1)
	{
		perror("utmp");
		return 1;
	}
	while(read(ufd, &u, sizeof u) == sizeof u)
	{
		if(u.u_u_name[0] != '\0' && u.u_u_id != 0)
		{
			close(ufd);
			return 1;
		}
	}
	close(ufd);
	return 0;
}
#endif
