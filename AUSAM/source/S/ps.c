#define	AUSAM
	/*
	 *
	 *	aug '77		Chris M, Ian J
	 *
	 *	consolidated ps
	 *	faster - gprocs - print time - no k flag -
	 *	better defintion of a & t flags
	 *
	 *	nov '77		Ian J
	 *
	 *	introduce 'c' and 'p' flags
	 *	'c'  => print sum of child usr+sys times
	 *	'p'  => print sum of usr+sys times
	 *	'cp' => print sum of child & parant times
	 *	default is 'p'
	 *	also check locations 54&56 for non-zero
	 *	if so this long indicates clock tics since
	 *	clock initialized. print this info atop listing.
	 *	allow 'u' and 's' flags
	 *	'u'  => print user time only
	 *	's'  => print system time only
	 *	'us' => (default) print sum of user and system times
	 *
	 */

/*
 *	ps - process status
 *	examine and print certain things about processes
 */

#include <param.h>
#include <proc.h>
#include <tty.h>
#include <user.h>

struct proc proc[NPROC];
int ttydev;
struct user u;
char partab[1];

int	lflg;
int	zflg;
int	cflg;
int	pflg;
int	uflg;
int	sflg;
int	xflg;
int	tflg;
int	gflg;
int	titflg;
int	aflg;
int	mem;		/* file desc. for kernal memory space */
int	swap;		/* file desc. for swap area */
char	ltty[64];	/* char id's for desired terminals */
char	*ptty ltty;	/* limit of id's for ltty */

int	ndev;
char	devc[65];	/* last character of tty name from dev */
int	devl[65];	/* corresponding major/minor number */
int	devt[65];

struct {
	long pusr;
	long psys;
	long cusr;
	long csys;
} tbuf;

struct ibuf {
	char	idevmin, idevmaj;
	int	inum;
	int	iflags;
#ifdef	AUSAM
	unsigned iuid;
#endif
	char	inl;
#ifndef	AUSAM
	char	iuid;
	char	igid;
#endif
	char	isize0;
	int	isize;
	int	iaddr[8];
	long	ictime;
	long	imtime;
#ifndef	AUSAM
	int	fill;
#endif
};
#define RSIZE 32
#define usagerr { printf("\n\t ps [-][alxcpuszt[c]]\n"); done(-1); }

main(argc, argv)
char **argv;
{
	struct proc *p;
	int nproc, b;
	register int i, c, n;
	char *ap, *comname;
	long comtime;
	unsigned uid, puid;
	long tics;
	double temp;
	extern fout;

	titflg = 1;		/* no title yet printed */
	fout = dup(1);
	if ( argc > 2 ) usagerr;
	if (argc > 1) {
		ap = argv[1];
		while (*ap) switch (*ap++) {
		case 'a':
			aflg++;
			break;

		case 't':
			tflg++;
			if( *ap ) *ptty++ = *ap++;
			break;

		case 'g':
			gflg++;
			break;

		case 'x':
			xflg++;
			break;

		case 'l':
			lflg++;
			break;

		case 'z':
			zflg++;
			break;

		case 'c':
			cflg++;
			break;

		case 'p':
			pflg++;
			break;

		case 's':
			sflg++;
			break;

		case 'u':
			uflg++;
			break;

		case '-':
			break;
		default:
			usagerr;

		}
	}
	if( (cflg==0)&&(pflg==0) ) pflg++;
	if( (uflg==0)&&(sflg==0) ) uflg++, sflg++;;

	if(chdir("/dev") < 0) {
		printf("Chdir /dev failed\n");
		done(-1);
	}
	if ((mem = open("mem", 0)) <= 0) {
		printf("No mem\n");
		done(-1);
	}
	getdev();
#ifndef	AUSAM
	uid = getuid();
#endif
#ifdef	AUSAM
	uid = getreal();
#endif
	if((swap = open("swap",0)) < 0)
		printf("No swap\n");
	if( (nproc=gprocs(proc)) > NPROC ) {
		printf(" tell a guru to re-compile PS.C \n");
		done(-1);
	}
	if ( zflg ) {
		seek(mem,054,0); read(mem,&tics,4);
		if( tics ) {
			ptime(tics);
			printf(" since boot");
			i = proc[0].p_addr;
			c = i>>3;
			i = (i&07)<<6;
			i =+ &(0->u_utime);
			seek(mem, c, 3);
			seek(mem, i, 1);
			read(mem,&tbuf,sizeof tbuf); /* get times for system process */
			temp = tbuf.pusr + tbuf.psys + tbuf.cusr + tbuf.csys;
			temp = 100 * temp / tics + 0.5;
			printf("\n     %2.0f%% idle time\n",temp);
		}
	}
	for (i=0; i<nproc; i++) {
		if (proc[i].p_stat==0)
			continue;
		if (proc[i].p_ttyp==0) {
			if (xflg==0)
				continue;
			c = '?';
		} else {
			for(c=0; c<ndev; c++)
			if(devt[c] == proc[i].p_ttyp) {
				c = devc[c];
				goto out;
			}
			seek(mem, &((proc[i].p_ttyp)->t_dev), 0);
			read(mem, &ttydev, 2);
			for(c=0; c<ndev; c++)
			if(devl[c] == ttydev) {
				devt[c] = proc[i].p_ttyp;
				c = devc[c];
				goto out;
			}
			c = '?';
		out:;
		}
#ifndef	AUSAM
		puid = proc[i].p_uid & 0377;
#endif
#ifdef	AUSAM
		puid = proc[i].p_uid;
#endif
		/*
		 *	four possibilities of which process's to look at:
		 *
		 *		    a=0		    a=1
		 *
		 *	t=0	all terminals	all terminals
		 *		own processes	all processes
		 *
		 *	t=1	own terminal	own terminal	*
		 *		own processes	all processes
		 *
		 *	*note:	default is own terminal but maybe the named
		 *		terminal
		 */
		if( aflg ) {
			if( tflg && destty(c) ) continue;
		} else {
			if( uid != puid ) continue;
			else if( tflg && destty(c) ) continue;
		}
		if( (comname=getcom(i,&comtime)) == 0 ) continue;
		if( titflg )
			if(lflg) {
#ifndef	AUSAM
	   		printf("TTY F S UID   PID PRI   ADDR  SZ  WCHAN   TIME  COMMAND\n");
#endif
#ifdef	AUSAM
	   		printf("TTY F S  UID   PID PRI   ADDR  SZ  WCHAN   TIME  COMMAND\n");
#endif
	   		} else printf("%s  PID   TIME  COMMAND\n",tflg==1 ? " " : "TTY");
		titflg = 0;
		if ( lflg || tflg!=1 || destty(c) )
			{ putchar(c); putchar(':'); }
		else if( tflg == 0 )
			{ putchar(' '); putchar(' '); }
		if (lflg) {
#ifndef	AUSAM
			printf("%3o %c%4d", proc[i].p_flag,
#endif
#ifdef	AUSAM
			printf("%3o %c%5d", proc[i].p_flag,
#endif
				"0SWRIZT"[proc[i].p_stat], puid);
		}
		printf("%6l", proc[i].p_pid);
		if (lflg) {
			if( proc[i].p_nice == 0) c = ' ';
			else if(proc[i].p_nice > 0) c = '+';
			      else c = '-';
			printf("%4d%c%6o%4d", proc[i].p_pri, c, proc[i].p_addr,
				(proc[i].p_size+7)>>3);
			if (proc[i].p_wchan)
				printf("%7o", proc[i].p_wchan); else
				printf("       ");
		}
		ptime( comtime ); /* output time used */
		printf(lflg?" %.17s\n":" %.62s\n", comname);
	}
	if( titflg ) printf("no procs found\n");
	done(0);
}

getdev()
{
	register struct {
			int dir_ino;
			char dir_n[14];
	} *p;
	register i, c;
	int f;
	int mdev;
	char *dbuf = &proc;
	int sdbuf = sizeof proc;
	int sbuf[20];

	if( (f = open("."))<0 ) {
		printf("cannot open /dev\n");
		done(-1);
	}
	c = 0;
	if( ptty == ltty ) {
		fstat( 0, sbuf);
		mdev = sbuf->iaddr[0];
	}

loop:
	if( (i = read(f, dbuf, sdbuf))<=0 ) {
		close(f);
		ndev = c;
		return;
	}
	for(p = dbuf; p < &dbuf[i]; p++) {
		if(p->dir_ino == 0)
			continue;
		if(p->dir_n[0] == 't' &&
		   p->dir_n[1] == 't' &&
		   p->dir_n[2] == 'y' &&
		   p->dir_n[4] == 0 &&
		   p->dir_n[3] != 0) {
			if(stat(p->dir_n, sbuf) < 0)
				continue;
			devc[c] = p->dir_n[3];
			devl[c] = sbuf->iaddr[0];
			if( (mdev == devl[c]) && (ptty==ltty) )
				*ptty++ = devc[c];
			c++;
			continue;
		}
	}
	goto loop;
}

getcom(i,comtime)
int i;
long *comtime;
{
	int baddr, laddr, mf;
	register int *ip,cp;
	register char *cp1;
	int c, nbad;
	long ct;
	static char stbuf[513];

	*comtime = -1;
	if( proc[i].p_stat==SZOMB ) return("<defunct>");
	baddr = 0;
	laddr = 0;
	if (proc[i].p_flag&SLOAD) {
		laddr = proc[i].p_addr;
		mf = mem;
	} else {
		baddr = proc[i].p_addr;
		if((mf = swap) < 0)
			return("<??>");
	}
	baddr =+ laddr>>3;
	laddr = (laddr&07)<<6;
	seek(mf, baddr, 3);
	laddr =+ &(0->u_utime);
	seek(mf,laddr,1);
	read(mf,&tbuf,sizeof tbuf);
	if(!uflg) tbuf.pusr = tbuf.cusr = 0;
	if(!sflg) tbuf.psys = tbuf.csys = 0;
	if(pflg) ct =  tbuf.pusr+tbuf.psys; else ct=0;
	if(cflg) ct =+ tbuf.cusr+tbuf.csys;
	*comtime = ct;	/* save accumulated time for later printout */
	if( i == 0 ) return("UNIX");
	baddr = 0; laddr = 0;
	if(proc[i].p_flag & SLOAD)
		laddr = proc[i].p_addr;
	else
		baddr = proc[i].p_addr;
	laddr =+ proc[i].p_size - 8;
	baddr =+ laddr>>3;
	laddr = (laddr&07) << 6;
	seek(mf,baddr,3);
	cp = laddr;
	if(mf == mem)		/* less reading */
		cp =+ 512-RSIZE;
	seek(mf,cp,1);
	cp =- laddr;
	ip = &stbuf[512];
	do {
		read(mf,stbuf + cp,( mf == mem ? RSIZE : 512));
		while(ip > (stbuf + cp))
			if( *--ip == -1) goto found;
		seek(mf, -(RSIZE<<1), 1);
		cp =- RSIZE;
	} while( cp >= 0);
	return("");
found:	cp1 = ++ip;
	nbad = 0;
	for (; cp1 < &stbuf[512]; cp1++) {
		c = *cp1;
		if (c==0)
			*cp1 = ' ';
		else if (c < ' ' || c > 0176) {
			if (++nbad >= 5) {
				*cp1++ = ' ';
				break;
			}
			*cp1 = '?';
		}
	}
	while (*--cp1==' ') *cp1 = 0;
	{
		char *q,*p;
		q = "getty";	p = ip;
		while( *q++ == *p++ ) if( (*q==0) && (*p==' ') && (gflg==0) ) return(0);
		if( i>1 ) {
			q = "init";	p = ip;
			while( *q++ == *p++ ) if( (*q==0) && (*p==0) && (gflg==0) ) return(0);
		}
	}
	return(ip);
}

ptime(tics)
long tics;
{
	struct {
		int hiword;
		int loword;
	};

	if( tics<0 )
	{
		printf("        ");
	}
	else
	{
		register divn,sepc,trac,cp;

		divn = 100;
		sepc = '.'; trac = 's';		/* sss.ss seconds */
		if(tics > 15000)
		{
			sepc = 'm';		/* xxxmxxs minutes */
			tics = tics / 50; divn = 60;
			if(tics > 18000)
			{
				sepc = 'h';trac = 'm';	/* xxxhxxm hours */
				tics = tics / 60;
				if(tics > 7200)
				{
					sepc = 'd'; trac = 'h';	/* xxxdxxh */
					tics = tics/60;
					divn = 24;
				}
			}
		}
		else tics = tics * 2;
		cp = tics.loword;
		printf(" %3l%c%c%c%c", cp/divn, sepc,
			(cp%divn)/10 + '0', (cp%divn)%10 + '0', trac);
	}
}

done(cc)
{

	flush();
	exit(cc);
}

destty( c )
register c;
{
	char *p = ptty;
	char *q = ltty;

	while( q < p )
		if( *q++ == c ) return 0;
	return 1;
}
