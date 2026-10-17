#
/*

	l p d  in a key of c

*/

#define	CATCH	2	/* report files as printed on tty8 */

#include <local-system>
#include <printers.h>
#define	lpddir	"/tmp/lpd"

 
 
 
 
	int	desp;		/* file desc. of lp */
	int	desl;		/* file desc. of lock file */
	int	lmsg[2];	/* This is buffer for the lock file */
	int	elp	0;	/* cancel print flag */
	int	repf	0;	/* print repeated */
	int	batch;		/* batch printout */
	int	prtrnum;	/* number of this printer */
	int	ffdone	1;	/* tells whether aligned on page or not */
	char	lock[]	"lock\0";
#	define	lockposn	lock[4]
	char	xdirp[]	"x";
#	define	xdir	xdirp[0]
	char	*prname	"/dev/lp\0";
#	define	pposn		prname[7]
	int	dirnam	'x/';
	struct	dirbuf {
			int d_ino;
			char d_name[14];
		} dbuf[2];
	int	ttybuf[3];

	char	m1[]	"\n\n\n *** print restarted ***\f";
	char	m2[]	"\n\n\n *** print cancelled ***\f";
#define NEJECT 4

int comdir;

eflag()
{
	signal(5,eflag);
	elp = 1;
	if( printer[prtrnum].baudrate == PARALLEL)
	{
#		define	FLUSH	040
		ttybuf[0] =| FLUSH;
		stty(desp, ttybuf);
		ttybuf[0] =& ~FLUSH;
	}
}
restart()
{
	signal(6, restart);
	elp = 2;
}
repeat()
{
	signal(7, repeat);
	repf++;
}

int waiting;

getgoing()
{
	signal(4,1);		/* ignore again */
	waiting --;
}

shutdown()
{
	if( waiting ) unlink(lock);
	exit(0);
}


comp(s1,s2)
char *s1,*s2;
{
	register char *c1,*c2;

	c1 = s1; c2 = s2;
	while( (*c1 == *c2) && *c1) c2++,c1++;
	return(*c1 - *c2);
}

next()
{
	register struct dirbuf *d1,*d2;
	register i;

	seek(comdir, 32, 0);	/* skip . .. */
	d1 = &dbuf[0];
	d2 = &dbuf[1];
	d1->d_ino = 0;
	while( read(comdir,d1,16))
		if(d1->d_ino) break;
	if(!d1->d_ino) return(0);
	while( read(comdir,d2,16))
	{
		if(!d2->d_ino) continue;
		if(comp( &(d1->d_name[2]), &(d2->d_name[2])) > 0)

		{
			d2->d_ino = d1;
			d1 = d2;
			d2 = d1->d_ino;
		}
	}
	d1->d_ino = dirnam;
	return( d1);
}

main(ac, av)
char *av[];
{
	register z,df;
	register char *buf;
	char	*dn;
	int desr,push,ncopies,ncsave,isban;
	int firsttime = 0;
	char lastban[20];
	char buff[512];


	signal(1,1); signal(2,1); signal(3,1); signal(4,1);
	signal(5,eflag); signal(6,restart); signal(7,repeat);
	signal(14,shutdown);

	for(z = 3; z < 16; close(z++));
	if( ac == 2)		/* lpd for alternate printer */
	{
		switch( av[1][0] )
		{
	    case '0':
			prtrnum = 0;
			break;		/* default */
	    case '1':
			prtrnum = 1;
			dirnam = '1/';
			xdir = pposn = lockposn = '1';
			break;
	    case '2':
			prtrnum = 2;
			dirnam = '2/';
			xdir = pposn = lockposn = '2';
			break;
	    case '3':
			prtrnum = 3;
			dirnam = '3/';
			xdir = pposn = lockposn = '3';
			break;
	    default:
			prints(2,"bad printer number\n");
			exit(-1);
		}
	}
	if( (chdir(lpddir) < 0) || ((comdir = open(xdirp, 0)) < 0) )
	{
		prints(2, "lpd directory screwed\n");
		return(1);
	}
	if( (z = open(lock, 2)) != -1)
	{
		if( read(z, &desp, 2) != 2 || kill(desp, 2) == -1)
		{
			seek(z, 0, 0);
		}
		else
		{
			prints(2, "lpd already active\n");
			return(1);
		}
	}
	else
	{
		z = creat(lock, 0);
	}
	if(fork())
	{
		return(0);
	}
	lmsg[0] = getpid();
	lmsg[1] = -1;
	write(z, lmsg, 4);
	desl = z;	/* daemon now ready to print */
 start:
	elp = 0; buf = buff;
	while((desp = open(prname,1)) < 0) sleep(10);
	gtty(desp, ttybuf);
	if(printer[prtrnum].baudrate != PARALLEL)		/* assume serial LA180 */
	{
		ttybuf[0] = printer[prtrnum].baudrate;
		ttybuf[2] = printer[prtrnum].modes;
		stty(desp, ttybuf);
	}
	while( dn = next() )
	{
begin:
		push = isban = batch = repf = 0;
		ncopies = ncsave = 1;
		df = open(dn, 0);
		while(read(df,buf,40) == 40 )
		{
			buf[40] = 0;
			ncopies = ncsave;
			switch( (z = buf[0] | 040))
			{
		    case 'i':			/* uid next */
				seek(desl, 2, 0);
				write(desl, &buf[1], 2);	/* KFH fix */
				break;
		    case 'f':
				if((desr = open( &buf[1], 0)) < 0)
					break;
				if( printer[prtrnum].baudrate != PARALLEL)
					firsttime = 1;
				do
				{
					if(firsttime)
						firsttime = 0;
					else
						prints(desp,"\014");
					seek(desr,0,0);
					while((z = read(desr,buf,512)) && !elp)
						write(desp,buf,z);
					if( repf)
					{
						ncopies++;
						ncsave++;
						repf--;
					}
					if(elp == 2)
					{
						elp = repf = 0;
						seek(df, 0, 0);
						prints(desp, m1);
						ncopies = ncsave;
						if( isban) banner(lastban);
					}
				}
				while( --ncopies && !elp);
				close(desr);
				break;
		    case 'b':	batch++;
		    case 'l':	banner(&buf[1]);
				isban++;
				z = 0;
				while( (z<20) && (lastban[z++] = buf[z]));
				lastban[z] = 0;
				break;
		    case 'u':	unlink(&buf[1]);
				break;
		    case 'n':	ncopies = atoi(&buf[1]);
				if(ncopies <= 0) ncopies = 1;
				ncsave = ncopies;
				break;
		    case 'p':	push++;
				break;
		    case 'm':	{
					static char ttyname[] "/dev/tty?";
					ttyname[8] = buf[1];
					z = open(ttyname,1);
					prints(z,"\007\n");
					prints(z,&buf[2]);
					prints(z,": lpd finished\n");
					close(z);
					break;
				}
		    case 'y':
				if( printer[prtrnum].baudrate == PARALLEL )		/* real line printer */
				{

#					define	NOEJECT	0200
					ttybuf[0] =| NOEJECT;
					stty(desp, ttybuf);
				}
			}
			if(elp)
			{
				if( printer[prtrnum].baudrate == PARALLEL)
				{
					prints(desp, "\n");
					stty(desp, ttybuf);	/* turn off flush */
				}
				prints(desp, m2);
				elp = 0;
			/*	break;	*/
			}
		}
		close(df);
		unlink(dn);
		if( printer[prtrnum].baudrate == PARALLEL)
		{
			ttybuf[0] =& ~NOEJECT;
			stty(desp, ttybuf);
		}
	}
	seek(desl, 0, 0);
	write(desl, lmsg, 4);
	sleep(2);
	waiting = 0;
	signal(4,getgoing);
	if(dn = next()) goto begin;
	if( push)
		write(desp,"\001\014\001\014\001\014\001\014\001\014\001\014",2*NEJECT);
	else
		if(!ffdone)
			write(desp,"\f",1);
	close( desp );
	ffdone = 1;	/* set flag to avoid form feed on LA180's */
	waiting ++;
	while( waiting > 0 ) sleep( 32767 );
	goto start;
}






	char m3[]
"\013\376\210\r\376\210\r\014\n\n\n\n\n\377\020unix edition-6 local print\n\n\377\060";
	char m4[]
"\377\000*****                     \n\n" ;
	char m5[]
"----------------------------------------\n";
	char header[]
"\n\n\t\t\t\tUnix Note\n\n\n";

banner(s)
char *s;
{
	register char *i,*j;

	if( printer[prtrnum].baudrate == PARALLEL)
	{
		prints( desp, m3);
		prints( desp, ijtime());
		write(desp,header,2);
		i = &m4[8];
		for(j = s; *j && i < &m4[28] ; ) *i++ = *j++;
		*i++ = '\n'; *i++ = '\n';
		for(m4[1] = 40; m4[1] < 52; m4[1]++)
			write(desp,m4,i-m4);
		note(batch ? "batchnote" : "unixnote");
		write(desp,m3,8);
	}
	else
	{
		if(!ffdone)
			prints(desp,"\f");
		ffdone = 0;
		prints( desp, s);
		prints( desp, "\t\t");
		prints( desp, ijtime());
		prints( desp, m5);
	}
#ifdef	CATCH
	switch( prtrnum )
	{
	case 0:	prints( CATCH , "0: ");
		break;
	case 1:	prints( CATCH , "1: ");
		break;
	}
	prints( CATCH , s );
	prints( CATCH , ijtime() );
#endif	CATCH
}



note(s)
char *s;
{
	int notebuf[256];
	register ndes,nbuf;

	nbuf = notebuf;
	if((ndes = open(s,0)) < 0) return;
	prints(desp,header);
	while( write(desp,nbuf,read(ndes,nbuf,512)) == 512 );
	close(ndes);
}
