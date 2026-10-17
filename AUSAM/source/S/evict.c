#
/*
 *	This program is used to remove ungracefully a group of persons
 *	from the system .
 *	Used by TERMBOOK . Can also be used directly .
 *	Syntax:
 *		evict ttys [ wait [ lname ] ]
 *
 *		'ttys' is a string of chars, 'ab...' meaning ttya,ttyb...
 *		'wait' the number of seconds between warning and death.
 *		'lname' is the person (or class) who has the right to remain.
 *
 *		Modified on the 29 june 1979 to give a log of eviction
 *		information. Defined as EVICTMONITOR in local-system.
 *							Graham S.
 */ 
#include	<param.h>
#include	<proc.h>
#include	<local-system>
#include	<class.h>
#include	<tty.h>
char	partab[1];
#include	<passwd.h>
#include	<lnode.h>
#include	<stat.h>
#include	<utmp.h>


#define	NVIC	70
#define NASTY	-40




#define		DIETIME		5
char	m0[]	"\nThis terminal is booked for ";
char	m1[]	"\nYou have    minutes to log off or your account\nwill be removed from the system.\nRemain off for at least 5 minutes.\n\tBe warned.\n";
char	*min	&m1[12];	/* where to put number of minutes */ 
char	m2[]	"account ";
char	m3[]	"members of class(es) ";
char	ttyn[]	"tty?";
#define	ttyx	ttyn[3]
struct	pwent	pe;	/* the pwent of he who may stay */ 
struct	proc	*hiproc;

int	nvic;		/* number of terminals evicted */
struct	victim
{
	int		 v_uid;
	struct	tty	*v_dev;
	struct	lnode	*v_lnp;
}
victims[NVIC];		/* one entry for every 1..nvic */
int	uids[256];	/* one uid per tty char */

int	class;		/* set when a class eviction in progress */
int	wte;		/* time allowed to log off in peace */
int	kmfd,utfd;	/* kmem, utmp file desciptors */

int	trycnt;	/* A KLUDGE SO THAT EVICT DOES NOT STAY AROUND TO LONG */



#ifdef EVICTMONITOR
#include <uid.h>
extern	fout;		/* A buffer for writing on the monitor file */
extern long	time();
int		thispid;
#endif EVICTMONITOR
main(argc, argv)
int argc;
char **argv;
{
	register char *s;
	register int   warned;

	nice(NASTY);

#ifdef EVICTMONITOR
	thispid = getpid();
	if((fout = open(EVICTLOG,2)) == -1)
		err("Can't open : %s\n",EVICTLOG);
	seek(fout,0,2);		/* Put it at the end of the file */
	printf("%ld %d ",time(),thispid);
		{
		unsigned	r;
		printf("Uid : %d",r = REALUID());
		if(argc > 1)
			printf(" Ttys : %s",argv[1]);
		if(argc > 2)
			printf(" Wait : %s",argv[2]);
		if(argc > 3)
			printf(" Lname : %s",argv[3]);
		printf("\n");
		}
	flush();

#endif EVICTMONITOR
	if(argc > 4 || argc < 2)
		err("Usage : evict ttys [ wait [ lname] ]\n");
	if(argc == 4)
	{
		pe.pw_strings[LNAME] = argv[3];
		if(getpwuid(&pe, 0, 0) < 0)
			err("Who ?\n");
		if(pe.pw_flags & CLASSACC)
			class++;
	}
	else
	{
		pe.pw_flags = 0;	/* just to be safe */ 
		pe.pw_uid = 0;
	}
	if(argc >= 3)
		wte = atoi(argv[2]);
	warned = wte/60;
	do
	{
		*--min = warned%10+'0';
		warned =/ 10;
	}
	while(warned);
	if(chdir("/dev") == -1)
		err("chdir /dev failed\n");
	if((kmfd = open("kmem", 0)) == -1)
		err("Can't open /dev/kmem\n");
	if((utfd = open("/etc/utmp", 0)) == -1)
		err("Can't open /etc/utmp\n");
	do
	{
		warned = 0;
		hiproc = &proc[gprocs(proc)];
		for(s = argv[1]; *s != 0; s++)
		{
			switch(whosat(*s))
			{
		    case 0:
				continue;
		    case 1:		/* to be warned */
				if(wte)
				{
					warn(*s, argc == 4 ? argv[3] : 0);
					warned++;
					break;
				}
		    case -1:
				zap(*s);
				break;
			}
		}
		/* The Kludge so that evict will go away */
		if (trycnt++ == 3) break;
		if(warned)
			sleep(wte);
	}
	while(warned);
	/*
	 * get the processes which did not submit to a SIGTERMINATE
	 */
	if(nvic)
		finalzap();
	else
		prints(2, "No evictions performed\n");
	return 0;
}

err(string)
char *string;
{
	prints(2, string);
	exit(1);
}

warn(c, name)
int c;
char *name;
{
	register int f;

#ifdef EVICTMONITOR
	seek(fout,0,2);
	printf("%ld %d Warn tty%c, %s\n",time(),thispid,c,name);
	flush();
#endif EVICTMONITOR
	ttyx = c;
	if((f = open(ttyn, 1)) >= 0)
	{
		if(class)
		{
			register int i;

			prints(f, &m0);
			prints(f, &m3);
			for(i = 0; i < (sizeof classes/sizeof classes[0]); i++)
			{
				if(pe.pw_cmask[classes[i].c_word]&classes[i].c_mask)
				{
					prints(f, classes[i].c_name);
					prints(f, " ");
				}
			}
		}
		else
		{
			if(name != 0)
			{
				prints(f, m0);
				prints(f, &m2);
				prints(f, name);
			}
		}
		prints(f, &m1);
		close(f);
	}
}

zap(c)
int c;
{
	register struct proc	*p;
	register struct tty	*dev	0;
	register struct lnode	*lnp;
	struct statb sb;
	int	ttydev;

	ttyx = c;
	stat(ttyn, &sb);
	if((sb.i_mode&IFMT) != IFCHR)
	{
		prints(2, ttyn);
		prints(2, ": not a character device.\n");
		return;
	}
	for(p = proc; p < hiproc; p++)
	{
		if(p->p_stat && p->p_stat != SZOMB && uids[c] == p->p_uid)
		{
			/*
			 * Look for ttyp and lnode.
			 */
			seek(kmfd, &(p->p_ttyp->t_dev), 0);
			read(kmfd, &ttydev, 2);
			if(ttydev == sb.i_addr[0])
			{
				dev = p->p_ttyp;
				lnp = p->p_lnode;
				victims[nvic].v_uid = uids[c];
				victims[nvic].v_dev = dev;
				victims[nvic].v_lnp = lnp;
#ifdef EVICTMONITOR
				seek(fout,0,2);
				printf("%ld %d tty%c, Lnode : 0%o\n",
					time(),thispid,c,lnp);
				flush();
#endif EVICTMONITOR
				nvic++;
				break;
			}
		}
	}
	if(dev)
	{
		for(p = proc; p < hiproc; p++)
		{
			if(p->p_stat
			    && p->p_stat != SZOMB
				&& p->p_ttyp == dev
				    && p->p_lnode == lnp)
			{
#ifdef EVICTMONITOR
				seek(fout,0,2);
				printf("%ld %d SIGTERMINATE Lnode : 0%o, Pid : %d\n",
					time(),thispid,lnp,p->p_pid);
				flush();
#endif EVICTMONITOR
				kill(p->p_pid, SIGTERMINATE);
			}
		}
	}
	else
	{
		prints(2, ttyx);
		prints(2, ": No procs found\n");
	}
}

whosat(c)
register c;
{
	struct utmp utmpst;
	struct pwent	ze;
	register int	 n;

	n = c;
	if(n >= '0' && n <= '9')
		n =- '0';
	else if(n >= 'a' && n <= 'z')
		n =+ 10-'a';
	else
		n =+ 36-'A';
	seek(utfd, UTMPSIZ*n, 0);
	read(utfd, &utmpst, UTMPSIZ);
	if(utmpst.u_ttyid == 0 || utmpst.u_u_id == 0 || pe.pw_uid == utmpst.u_u_id)
	{
		uids[c] = 0;
		return 0;
	}
	if(uids[c] == utmpst.u_u_id)
	{
		return -1;
	}
	ze.pw_uid = utmpst.u_u_id;
	getpwlog(&ze, 0, 0);
	if(ze.pw_flags & NOEVICT)
	{
		uids[c] = 0;
		return 0;
	}
	if(class)
	{
		register i;

		for(i = 0; i < CMASKSIZE; i++)
		{
			if(pe.pw_cmask[i] & ze.pw_cmask[i])
			{
				uids[c] = 0;
				return 0;
			}
		}
	}
	uids[c] = utmpst.u_u_id;
	return 1;
}

finalzap()
{
	register struct proc *p;
	register struct victim *v;
	struct	pwent	ze;
	char	names[SSIZ];

	sleep(DIETIME);
	hiproc = &proc[gprocs(proc)];
	for(v = victims; v < victims+nvic; v++)
	{
		for(p = proc; p < hiproc; p++)
		{
			if(p->p_stat
			   && p->p_stat != SZOMB
			      && p->p_lnode == v->v_lnp
				 && p->p_ttyp == v->v_dev)
			{
#ifdef EVICTMONITOR
				seek(fout,0,2);
				printf("%ld %d SIGKIL Lnode : 0%o, Pid : %d\n",
					time(),thispid,v->v_lnp,p->p_pid);
				flush();
#endif EVICTMONITOR
				kill(p->p_pid, SIGKIL);

			}
		}
		ze.pw_uid = v->v_uid;
#ifdef EVICTMONITOR
		if(getpwlog(&ze, names, sizeof names) == -1)
			{
			seek(fout,0,2);
			printf("%ld %d GETPWLOG FAILED\n",time(),thispid);
			flush();
			}
#else
		getpwlog(&ze, names, sizeof names);
#endif EVICTMONITOR
		ze.pw_tmask = 0;
		ze.pw_cmask[CMASKSIZE-1] =| 01;	/* set EVICTED flag */
		chmod(ze.pw_strings[DIRPATH], 0);
#ifdef EVICTMONITOR
		printf("%ld %d Chmod %s\n",time(),thispid,ze.pw_strings[DIRPATH]);
		seek(fout,0,2);
		printf("%ld %d EVICTED Uid : %d\n",time(),thispid,ze.pw_uid);
		flush();
#endif EVICTMONITOR
#ifdef EVICTMONITOR
		if(updtpwent(&ze) <= 0)
			{
			seek(fout,0,2);
			printf("%ld %d UPDTPWENT FAILED\n",time(),thispid);
			}
#else
		updtpwent(&ze);
#endif EVICTMONITOR
	}
}
