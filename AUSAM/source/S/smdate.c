#
/*
 *	smdate [[[[yy]mm]dd]hh]mm file [ ... file ]
 *
 *	Peter Ivanov	April '79
 *		much stolen from date.c by PDL.
 *		Still needs work. Messy to always reconvert first
 *		arg for each file.
 *
 */ 

#include	<local-system>
#include	<stat.h>

#ifdef	AUSAM

long timbuf;
int dmsize[];
char *ctime();
char *cbp;
struct statb stats;

main(argc, argv)
int argc;
char **argv;
{
	register int i;


	if(argc >= 3)
	{
		argc =- 2;
		for(i=2; argc--; i++)
			{
			if(stat(argv[i], &stats) == -1)
				perror(argv[i]);
			else
				{
				timbuf = stats.i_mtime;
				cbp = argv[1];
				if(gtime())
					{
					prints(2, "Bad conversion\n");
					return 1;
					}
				if(smdate(argv[i], timbuf) == -1)
						perror(argv[i]);
				}
			}
	}
  else
	{
		prints(2, "Usage: smdate [[[[yy]mm]dd]hh]mm file [ ... file ]\n");
		return 1;
	}

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
#endif
