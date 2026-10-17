#

/***
 *** Temporary vversion without nlist()
 ***    - Location x'400' in /dev/mem is used to locate the proc table
 ***      in memory.
 ***	- Also, "/dev/swap" must be an alias of the swap device.
 ***/


/*
 *	ps - process status
 *	examine and print certain things about processes
 */

#include "/usr/sys/param.h"
#include "/usr/sys/proc.h"
#include "/usr/sys/tty.h"
#include "/usr/sys/user.h"

struct namelist {
	char name[8];
	int  type;
	char  *value;
} nl[3];

struct proc proc[NPROC];
struct tty tty;
struct user u;
int	lflg;
int	kflg;
int	xflg;
int	tflg;
int	aflg;
int	mem;
int	swap;

int	stbuf[129];		/***/
int	ndev;
char	devc[65];
int	devl[65];
int	devt[65];
char	*coref;
struct ibuf {
	int	idev;		/***/
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
int	obuf[131];		/***/


main(argc, argv)
char **argv;
{
	struct proc *p;
	int n, b;
	int i, c, mtty;
	char *ap;
	int uid, puid;

	obuf[0] = 1;
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
	setup(&nl[1], "_swapdev");
	nlist(argc>2? argv[2]:"/unix", nl);
	if (nl[0].type==0) {
		printf("No namelist\n");
		return;
	}
	coref = "/dev/mem";
	if(kflg)
		coref = "/usr/sys/core";
	if ((mem = open(coref, 0)) < 0) {
		printf("No mem\n");
		done();
	}
 /** locate addr of proc table from mem **/
	seek(mem, 0x400, 0);
	read(mem, &nl[0].value, 4);
 
	seek(mem, nl[1].value, 0);
	read(mem, &nl[1].value, 4);		/***/
	seek(mem, nl[0].value, 0);
	read(mem, proc, sizeof proc);
	getdev();
	mtty = ttyn(0);
	uid = getuid() & 0377;
	if(lflg)
	printf("TTY F S UID   PID PRI  ADDR  SZ WCHAN COMMAND\n"); else	/***/
		printf("TTY  PID COMMAND\n");
	for (i=0; i<NPROC; i++) {
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
			seek(mem, proc[i].p_ttyp, 0);
			read(mem, &tty, sizeof tty);
			for(c=0; c<ndev; c++)
			if(devl[c] == tty.t_dev) {
				devt[c] = proc[i].p_ttyp;
				c = devc[c];
				goto out;
			}
			c = '?';
		out:;
		}
		puid = proc[i].p_uid & 0377;
		if (uid != puid && aflg==0)
			continue;
		if (lflg || c!=mtty)
			printf("%c:", c);
		else
			printf("  ");
		if (lflg) {
			printf("%3o %c%4d", proc[i].p_flag,
				"0SWRIZT"[proc[i].p_stat], puid);
		}
		printf("%6d", proc[i].p_pid);		/***/
		if (lflg) {
			printf("%4d%6x%4d", proc[i].p_pri, proc[i].p_addr,
				(proc[i].p_size+3)>>2);	/***/
			if (proc[i].p_wchan)
				printf("%6x", proc[i].p_wchan); else	/***/
				printf("       ");
		}
		if (proc[i].p_stat==5)
			printf(" <defunct>");
		else
			prcom(i);
		printf("\n");
	}
	done();
}

getdev()
{
	register struct { char dir_ino[2]; char dir_n[14]; } *p;
	register i, c;
	int f;
	char dbuf[512];
	int sbuf[20];


	f = open("/dev", 0);
	if(f <0) {
		printf("cannot open /dev\n");
		done();
	}
/***
	swap = -1;
***/
	swap = open("/dev/swap", 0);
	if (swap < 0) {
		printf("Can't find /dev/swap\n");
		done();
	}
	c = 0;

loop:
	i = read(f, dbuf, 512);
	if(i <= 0) {
		close(f);
/***
		if(swap < 0) {
			printf("no swap device\n");
			done();
		}
***/
		ndev = c;
		return;
	}
	while(i < 512)
		dbuf[i++] = 0;
	for(p= dbuf; p < dbuf+512; p++) {
		if((p->dir_ino[0] | p->dir_ino[1]) == 0)	/***/
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
/***
		if(swap >= 0)
			continue;
		if(stat(p->dir_n, sbuf) < 0)
			continue;
		if((sbuf->iflags & 060000) != 060000)
			continue;
		if(sbuf->iaddr[0] == nl[1].value)
			swap = open(p->dir_n, 0);
***/
	}
	goto loop;
}

setup(p, s)
char *p, *s;
{
	while (*p++ = *s++);
}

prcom(i)
{
	int baddr, laddr, mf;
	register int *ip;
	register char *cp, *cp1;
	int c, nbad;

	baddr = 0;
	if (proc[i].p_flag&SLOAD) {
		baddr = proc[i].p_addr;		/***/
		mf = mem;
	} else {
		baddr = proc[i].p_addr<<1;	/***/
		mf = swap;
	}
	baddr =+ proc[i].p_size - 2;		/***/
	baddr =<< 8;				/***/
	seek(mf, baddr, 0);			/***/
	if (read(mf, stbuf, 512) != 512)
		return(0);
	for (ip = &stbuf[128]; ip > &stbuf[0];) {		/***/
		if (*--ip == -1) {
			cp = ip+1;
			if (*cp==0)
				cp++;
			nbad = 0;
			for (cp1 = cp; cp1 < &stbuf[128]; cp1++) {	/***/
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

done()
{

	fflush(obuf);
	exit();
}

putchar(c)
{

	putc(c, obuf);
}

/*** dummy nlist to patch in address of proc ***/
nlist(fd, np)
struct namelist *np;
{
	np[0].type = 1;
}
