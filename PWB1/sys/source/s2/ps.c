#

/*
 *	ps - process status
 *	examine and print certain things about processes
 */

#include "param.h"
#include "proc.h"
#include "tty.h"
#include "user.h"
#include "var.h"

struct {
	char name[8];
	int  type;
	char  *value;
} nl[3];

struct	proc proc[1];
struct var v;

int	ua[256];
int	chkpid;
int	lflg;
int	kflg;
int	xflg;
int	tflg;
int	aflg;
int	mem;
int	swmem;
int	swap;

int	ndev;
char	devc[65];
int	devl[65];
char	*coref;
struct ibuf {
	char	idevmin, idevmaj;
	int	inum;
	int	iflags;
	char	inl;
	char	iuid;
	char	igid;
	char	isize0;
	int	isize;
	int	iaddr[8];
	char	*ictime[2];
	char	*imtime[2];
	int	fill;
};
char	partab[1];


main(argc, argv)
char **argv;
{
	struct proc *p;
	int n, b;
	register i;
	int	c, lim;
	char *ap;
	int uid, puid;
	extern fout;

	if (argc>1) {
		ap = argv[1];
		while (*ap) switch (*ap++) {
		case 'a':
			aflg++;
			break;

		case 't':
			if(*ap)
				tflg = *ap++;
			aflg++;
			if(tflg == '?')
				xflg++;
			break;

		case 'x':
			xflg++;
			break;

		case '-':
			break;

		case 'l':
			lflg++;
			break;

		case 'k':
			kflg++;
			break;

		default:
			chkpid=atoi(--ap);
			*ap = '\0';
			break;
		}
	}

	if(chdir("/dev") < 0) {
		write(2,"cannot change to /dev\n",22);
		done(1);
	}
	setup(&nl[0], "_proc");
	setup(&nl[1], "_swapdev");
	setup(&nl[2], "_v");
	nlist(argc>2? argv[2]:"/unix", nl);
	if (nl[0].type==0) {
		write(2,"No namelist\n",12);
		done(1);
	}
	coref = "/dev/mem";
	if(kflg)
		coref = "/sys/sys/core";
	if ((mem = open(coref, 0)) < 0) {
		write(2,"No mem\n",7);
		done(1);
	}
	swmem = open(coref, 0);
	/*
	 * read mem to find swap dev.
	 */
	seek(mem, nl[1].value, 0);
	read(mem, &nl[1].value, 2);
	seek(mem, nl[2].value, 0);
	read(mem, &v, sizeof v);
	/*
	 * Locate proc table
	 */
	seek(mem, nl[0].value, 0);
	getdev();
	uid = getuid() & 0377;
	fout = dup(1);
	if (lflg)
	printf(" F S UID   PID  PPID CPU PRI NICE ADDR  SZ  WCHAN TTY TIME COMMAND\n"); else
		if (chkpid==0) printf("   PID TTY TIME COMMAND\n");
	lim = (v.ve_proc-nl[0].value)/sizeof proc;
	for (i=0; i<lim; i++) {
		read(mem, proc, sizeof proc);
		if (proc[0].p_stat==0)
			continue;
		if (proc[0].p_pgrp==0 && xflg==0)
			continue;
		puid = proc[0].p_uid & 0377;
		if ((uid != puid && aflg==0) ||
		    (chkpid!=0 && chkpid!=proc[0].p_pid))
			continue;
		if(prcom(puid))
			printf("\n");
		flush();
	}
	done(0);
}

getdev()
{
	register struct { int dir_ino; char dir_n[14]; } *p;
	register i, c;
	int f;
	char dbuf[512];
	int sbuf[20];

	f = open("/dev", 0);
	if(f < 0) {
		write(2,"cannot open /dev\n",16);
		done(1);
	}
	swap = -1;
	c = 0;

loop:
	i = read(f, dbuf, 512);
	if(i <= 0) {
		close(f);
		if(swap < 0) {
			write(2,"no swap device\n",15);
			done(1);
		}
		ndev = c;
		return;
	}
	while(i < 512)
		dbuf[i++] = 0;
	for(p = dbuf; p < dbuf+512; p++) {
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
			c++;
			continue;
		}
		if(swap >= 0)
			continue;
		if(stat(p->dir_n, sbuf) < 0)
			continue;
		if((sbuf->iflags & 060000) != 060000)
			continue;
		if(sbuf->iaddr[0] == nl[1].value)
			swap = open(p->dir_n, 0);
	}
	goto loop;
}

setup(p, s)
char *p, *s;
{
	while (*p++ = *s++);
}

prcom(puid)
{
	int baddr, laddr, mf;
	register int *ip;
	register char *cp, *cp1;
	long tm;
	int c, nbad;

	baddr = 0;
	laddr = 0;
	if (proc[0].p_flag&SLOAD) {
		laddr = proc[0].p_addr;
		mf = swmem;
	} else {
		baddr = proc[0].p_addr;
		mf = swap;
	}
	baddr =+ laddr>>3;
	laddr = (laddr&07)<<6;
	seek(mf, baddr, 3);
	seek(mf, laddr, 1);
	if (read(mf, &ua[0], 512) != 512)
		return(0);
	c = gettty();
	if(tflg && c != tflg)
		return(0);
	if (lflg) {
		printf("%2o %c%4d", proc[0].p_flag,
			"0SWRIZT"[proc[0].p_stat], puid);
	}
	printf("%6u", proc[0].p_pid);
	if (lflg) {
		printf("%6u%4d%4d%5d%5o%4d", proc[0].p_ppid, proc[0].p_cpu&0377,
			proc[0].p_pri,
			proc[0].p_nice,
			proc[0].p_addr, (proc[0].p_size+7)>>3);
		if (proc[0].p_wchan)
			printf("%7o", proc[0].p_wchan); else
			printf("       ");
	}
	printf("  %c", c);
	if (proc[0].p_stat==5) {
		printf("  <defunct>");
		return(1);
	}
	tm = (ua[0].u_utime + ua[0].u_stime + 30)/60;
	printf(" %2ld:", tm/60);
	tm =% 60;
	printf(tm<10?"0%ld":"%ld", tm);
	if (0 && lflg==0) {	/* 0 == old tflg (print long times) */
		tm = (ua[0].u_cstime + 30)/60;
		printf(" %2ld:", tm/60);
		tm =% 60;
		printf(tm<10?"0%ld":"%ld", tm);
		tm = (ua[0].u_cutime + 30)/60;
		printf(" %2ld:", tm/60);
		tm =% 60;
		printf(tm<10?"0%ld":"%ld", tm);
	}
	c = proc[0].p_size - 8;
	laddr =+ (c&07)<<6;
	baddr =+ c>>3;
	seek(mf, baddr, 3);
	seek(mf, laddr, 1);
	if (read(mf, ua, 512) != 512)
		return(1);
	for (ip = &ua[256]; ip > &ua[0];) {
		if (*--ip == -1) {
			cp = ip+1;
			if (*cp==0)
				cp++;
			nbad = 0;
			for (cp1 = cp; cp1 < &ua[256]; cp1++) {
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
			while (*--cp1==' ')
				*cp1 = 0;
			printf(lflg?" %.40s":" %.80s", cp);
			return(1);
		}
	}
	return(1);
}

gettty()
{
	register i;

	if (ua[0].u_ttyp==0)
		return('?');
	for (i=0; i<ndev; i++)
		if (devl[i] == ua[0].u_ttyd)
			return(devc[i]);
	return('?');
}

done(exitno)
{
	flush();
	exit(exitno);
}
