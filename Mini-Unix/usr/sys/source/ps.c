#

/*
 *	ps - process status
 *	examine and print certain things about processes
 */

#include "/usr/sys/param.h"
#include "/usr/sys/proc.h"
#include "/usr/sys/tty.h"
#include "/usr/sys/user.h"

struct {
	char name[8];
	int  type;
	char  *value;
} nl[3];

struct proc proc[1];
int	ua[256];
int	lflg;
int	kflg;
int	xflg;
int	tflg;
int	aflg;
int	mem;
int	swmem;
int	swap;
int	swapdev;

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


main(argc, argv)
char **argv;
{
	struct proc *p;
	int n, b;
	int i, c;
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
			tflg++;
			break;

		case 'x':
			xflg++;
			break;

		case 'l':
			lflg++;
			break;

		case 'k':
			kflg++;
			break;

		}
	}

	if(chdir("/dev") < 0) {
		printf("cannot change to /dev\n");
		done();
	}
	setup(&nl[0], "_proc");
	nlist(argc>2? argv[2]:"/mx", nl);
	if (nl[0].type==0) {
		printf("No namelist\n");
		done();
	}
	coref = "/dev/mem";
	if(kflg)
		coref = "/usr/sys/kore";
	if ((mem = open(coref, 0)) < 0) {
		printf("No mem\n");
		done();
	}
	swmem = open(coref, 0);
	/*
	 * read mem to find swap dev.
	 */
	swapdev = SWAPDEV;
	/*
	 * Locate proc table
	 */
	seek(mem, nl[0].value, 0);
	getdev();
	uid = getuid() & 0377;
	fout = dup(1);
	if(lflg)
	printf("F  S UID   PID CPU PRI  ADDR  SZ WCHAN TTY TIME COMMAND\n"); else
		printf("  PID TTY TIME COMMAND\n");
	for (i=0; i<NPROC; i++) {
		read(mem, proc, sizeof proc);
		if (proc[0].p_stat==0)
			continue;
		if (proc[0].p_pgrp==0 && xflg==0)
			continue;
		puid = proc[0].p_uid & 0377;
		if (uid != puid && aflg==0)
			continue;
		if (lflg) {
			printf("%2o %c%4d", proc[0].p_flag,
				"0SWRIZT"[proc[0].p_stat], puid);
		}
		printf("%6l", proc[0].p_pid);
		if (lflg) {
			printf("%4d%4d%6o%4d", proc[0].p_cpu&0377, proc[0].p_pri,
			  proc[0].p_pid*SWPSIZ + SWPLO,
			    proc[0].p_size);
			if (proc[0].p_wchan)
				printf("%7o", proc[0].p_wchan); else
				printf("       ");
		}
		prcom(proc[0].p_stat);
		printf("\n");
		flush();
	}
	done();
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
		printf("cannot open /dev\n");
		done();
	}
	swap = -1;
	c = 0;

loop:
	i = read(f, dbuf, 512);
	if(i <= 0) {
		close(f);
		if(swap < 0) {
			printf("no swap device\n");
			done();
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
		if(sbuf->iaddr[0] == swapdev)
			swap = open(p->dir_n, 0);
	}
	goto loop;
}

setup(p, s)
char *p, *s;
{
	while (*p++ = *s++);
}

prcom(stat)
{
	int baddr, laddr, mf;
	register int *ip;
	register char *cp, *cp1;
	int c, nbad;

	baddr = 0;
	laddr = 0;
	if (proc[0].p_flag&SLOAD) {
		laddr = (TOPSYS>>6) - 16;
		mf = swmem;
	} else {
		baddr = proc[0].p_pid*SWPSIZ + SWPLO;
		mf = swap;
	}
	baddr =+ laddr>>3;
	laddr = (laddr&07)<<6;
	seek(mf, baddr, 3);
	seek(mf, laddr, 1);
	if (read(mf, &ua[0], 512) != 512)
		return(0);
	printf(" %c", gettty());
	if (stat==5) {
		printf("  <defunct>");
		return;
	}
	c = ((ua[0].u_utime>>1)&077777);
	c =+ ((ua[0].u_stime>>1)&077777);
	c = ldiv(0, c, 30);
	printf(" %2d:", c/60);
	c =% 60;
	printf(c<10?"0%d":"%d", c);
	if (proc[0].p_flag&SLOAD)
		c = (SWPSIZ<<3) - 8;
	else
		c = proc[0].p_size - 8;
	laddr =+ (c&07)<<6;
	baddr =+ c>>3;
	seek(mf, baddr, 3);
	seek(mf, laddr, 1);
	if (read(mf, ua, 512) != 512)
		return(0);
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
			printf(lflg?" %.16s":" %.64s", cp);
			return(1);
		}
	}
	return(0);
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

done()
{
	flush();
	exit();
}
