/*
 *	Big-brother-is-watching-YOU program.
 *	Prints out procs that are currently active on the given device,
 *	or accessing the given file.  Options allow for killing those procs,
 *	exec'ing chk, and sorting the output.
 *
 *	The program assumes that raw devices live in /dev (for chk),
 *	and are formed by adding 'r' to the block device name.
 *
 *				Kevin Hill
 */

#include	<local-system>
#include	<stat16.h>
#define	IFCHR	020000
#include	<param.h>
#include	<proc.h>
#include	<user.h>

#define	NTEXT	1			/* as otherwise the next 3 includes */
#define	NFILE	1			/*	declare rather large arrays */
#define	NINODE	1
#include	<text.h>
#include	<file.h>
#include	<inode.h>
#include	<tty.h>
char partab[1];				/* declared in tty.h */
#include	<param.h>		/* restore NTEXT NFILE NINODE */


int	inodes[NINODE];			/* store inumber of actual file */
int	files[NFILE];
int	texts[NTEXT];

#define	SIG	14			/* sent to procs if k-flag set */

#ifdef	EECF1140
#define	TTYS	20			/* max. nr. ttys in /dev */
#else
#define	TTYS	65			/* max. nr. ttys in /dev */
#endif	EECF1140

char ttyc[TTYS + 1];			/* tty char */
int  ttyd[TTYS + 1];			/* tty dev */

int  ttyp[TTYS + 1];			/* p->ttyp */
int  ttyi[TTYS + 1];			/* index into ttyc & ttyd */

#define	NI	40			/* max nr. args to chk */
int	list[NI], li;

char	mem[] "mem";
int	memfd;
char	swap[] "swap";
int	swapfd;
char	devdir[] "/dev";
int	devfd;
char	unix[] "/unix";
char	sort[] "/bin/sort";
#define	SORT	5			/* offset to skip bin above */
char	cant[] "Can't find %s\n";
char	warning[] "\007Warning: too many %s: recompile\n";
#ifdef	EECF
char	chk[] "/etc/bin/chk";
#define	CHK	9			/* offset to skip etc bin above */
#else
char	chk[] "/etc/chk";
#define	CHK	5
#endif	EECF

int	dev, inum;
int	heading, col;			/* output listing controls */
int	pid, uid;
struct tty *tty;
char	blkfile[16] "r";		/* block-file name corresponding to given arg */
int killflag, chkflag, fileflag, sortflag;

char args[256];

struct { unsigned unsd; };
struct { char *ptr; };

extern fout;

struct nl
{
	int name[4];
	int type;
	unsigned value;
}
nl[]
{
	'_i', 'no', 'de',  '\0\0', 0, 0,
#define	PINODE	0
	'_f', 'il', 'e\0', '\0\0', 0, 0,
#define	PFILE	1
	'_t', 'ex', 't\0', '\0\0', 0, 0,
#define	PTEXT	2
	'_p', 'ro', 'c\0', '\0\0', 0, 0,
#define	PPROC	3
	0
};

main(ac, av)
register char **av;
{
	register char *cp;
	register struct nl *nlp;

	fout = 1;
	while (av[1][0] == '-')
	{
		switch((*++av)[1])
		{
	    case 'f':	fileflag++; break;

	    case 'k':	printf("\n\tk flag: are you EXTREMELY sure?? ");
			if ((cp = getchar()) == 'y') killflag++;
			if (cp && cp != '\n') while (getchar() != '\n');
			break;

	    case 'n':	chkflag++; break;

	    case 's':	sortflag++; break;
		}
		ac--;
	}
	if (ac != 2)
	{
		printf("Usage: look [-f][-k][-n][-s] file\n");
		return 1;
	}
	if (newstat(cp = *++av, args) < 0 || chdir(cp = devdir) < 0 ||
		((memfd = open(cp = mem, 0)) < 0) ||
		((swapfd = open(cp = swap, 0)) < 0) ||
		((devfd = open(cp = devdir, 0)) < 0))
	{
		printf(cant, cp);
		return 1;
	}
	inum = args->sb_inumber;
	dev = args->sb_major << 8 | (args->sb_minor & 0377);
	if (fileflag == 0)
		if ((args->sb_flags & IFTYP) == IFTYP)
		{
			cp = av[0];
			dev = args->sb_addr[0];
			while ((blkfile[++col] = *cp++) && col < 14)
				if ((blkfile[col]) == '/') col = 0;
			blkfile[col + 1] = 0;
			col = 0;
		}
		else if (inum != 1)
		{
			printf("%s: illegal file\n", *av);
			return 1;
		}
	initdev();
	nlist(unix, nl);
	for (nlp = nl; nlp->name[0]; nlp++)
		if (nlp->type == 0)
		{
			printf("Bad namelist\n");
			return 1;
		}
	nice(-60);
	setexit();	/* reset from `scanprocs' if inconsistent user found */
	heading = li = 0;
	scaninodes();
	scanprocs();
	nice(-10);
	if (heading)
	{
		flush();
		close(fout);
		if (sortflag) while (waitx(&sortflag) != -1);
		fout = 1;
		flush();
		if (chkflag && li)
		{
			if (blkfile[1] == 0)
			{
				printf("Can't match dev %d/%d\n", (dev >> 8) & 0377, dev & 0377);
				exit(1);
			}
			if (newstat(blkfile, args) < 0) callchk(&blkfile[1]);
			else callchk(blkfile);
		}
	}
	else if (! killflag) printf("Inode active, yet no procs found??\n");
	return 0;
}


initdev()
{
	register n, *ip;
	register char *cp;
	int buf[256];

	while ((n = read(devfd, buf, sizeof buf)) > 0)
	{
		for (ip = buf; ip != &buf[n]; ip =+ 8)
		{
			cp = &ip[1];
			if (*ip == 0 || ip[1] == 'sw' && ip[2] == 'ap' && cp[4] == 0) continue;
			if (ip[1] == 'tt' && cp[2] == 'y' && cp[4] == 0)
			{
				if (newstat(ip + 1, args) < 0 || (args->sb_flags & IFTYP) != IFCHR) continue;
				ttyc[li] = cp[3];
				ttyd[li] = args->sb_addr[0];	/* major & minor tty numbers */
				if (++li >= TTYS)
				{
					printf(warning, "ttys");
					exit(1);
				}
				continue;
			}
			if (blkfile[1] == 0 && newstat(ip + 1, args) >= 0 &&
					(args->sb_flags & IFTYP) == IFTYP && args->sb_addr[0] == dev)
			{
				col = 1;
				while ((blkfile[col] = *cp++) && col++ < 14);
				blkfile[col] = 0;
				col = 0;
			}
		}
	}
	ttyd[li] = -1;		/* end-of-list sentinel */
	li = 0;
}


scaninodes()
{
	register i, j;

	j = 1;
	seek(memfd, nl[PINODE].value, 0);
	for (i = 0; i < NINODE; i++)
	{
		read(memfd, inode, 6);
		if (inode->i_count != 0 && inode->i_dev == dev &&
			(! fileflag || inode->i_number == inum))
		{
			j = 0;
			inodes[i] = inode->i_number;
		}
		seek(memfd, sizeof inode[0] - 6, 1);
	}
	if (j)
	{
		printf("No active inode(s) found\n");
		exit(0);
	}
	seek(memfd, &(nl[PTEXT].value.ptr->x_iptr), 0);
	for (i = 0; i < NTEXT; i++)
	{
		read(memfd, &(text->x_iptr), 3);
		if (text->x_count != 0)
			texts[i] = inodes[(text->x_iptr.unsd - nl[PINODE].value)/sizeof inode[0]];
		seek(memfd, sizeof text[0] - 3, 1);
	}
	seek(memfd, nl[PFILE].value, 0);
	for (i = 0; i < NFILE; i++)
	{
		read(memfd, file, sizeof file[0]);
		if (file->f_count != 0)
			files[i] = inodes[(file->f_inode.unsd - nl[PINODE].value)/sizeof inode[0]];
	}
}


scanprocs()
{
	register struct proc *p;
	register i, j;

	if ((i = gprocs(proc)) >= NPROC)
	{
		printf(warning, "procs");
		exit(1);
	}
	p = &proc[i];
	while (--p >= proc)
	{
		if (p->p_stat == 0 || p->p_stat == SZOMB) continue;
		i = (p->p_textp.unsd - nl[PTEXT].value) / sizeof text[0];
		pid = p->p_pid;
		uid = p->p_uid;
		tty = p->p_ttyp;
		if (p->p_textp != 0 && texts[i])
		{
			if (killflag) { kill(p->p_pid, SIG); continue; }
			print(1, texts[i]);
		}
		readuser(p);
		i = (u.u_procp.unsd - nl[PPROC].value) / sizeof proc[0];
		if (i != p - proc)
		{				/* Inconsistent: p->p_addr->u_procp != p */
			if (sortflag)
			{
				kill(sortflag, 9);
				close(fout);
				fout = 1;
				flush();
			}
			printf("Retrying...\n");
			reset();
		}
		i = (u.u_cdir.unsd - nl[PINODE].value) / sizeof inode[0];
		if (inodes[i])
		{
			if (killflag) { kill(p->p_pid, SIG); continue; }
			print(2, inodes[i]);
		}
		for (i = 0; i < NOFILE; i++)
		{
			if (u.u_ofile[i] == 0) continue;
			j = (u.u_ofile[i].unsd - nl[PFILE].value) / sizeof file[0];
			if (files[j])
			{
				if (killflag) { kill(p->p_pid, SIG); continue; }
				print(3, files[j]);
			}
		}
		if (col) { putchar('\n'); col = 0; }
	}
}


readuser(p)
register struct proc *p;
{
	long ln, useraddr;

	if (p->p_flag & SLOAD)
	{
		useraddr = p->p_addr;
		useraddr =<< 6;
		ln = &(0->u_procp);
		ln =+ useraddr;
		lseek(memfd, ln, 0);
		read(memfd, &u.u_procp, 2);
		ln = &(0->u_cdir);
		ln =+ useraddr;
		lseek(memfd, ln, 0);
		read(memfd, &u.u_cdir, 2);
		ln = &(0->u_ofile[0]);
		ln =+ useraddr;
		lseek(memfd, ln, 0);
		read(memfd, &u.u_ofile[0], NOFILE * 2);
	}
	else
	{
		seek(swapfd, p->p_addr, 3);
		read(swapfd, &u, sizeof u);
	}
}


print(newcol, inum)
register newcol;
{
	register char *cp;
	int fdes[2];

	if (heading == 0)
	{
		printf("TTY  PID   UID  TEXT  CDIR  FILES ...\n");
		heading++;
		if (sortflag)
		{
			pipe(fdes);
			if ((sortflag = fork()) < 0)
			{
				close(fdes[0]);
				close(fdes[1]);
				fout = dup(1);
				printf(cant, sort);
				sortflag = 0;
			}
			else if (sortflag == 0)		/* child */
			{
				nice(-10);
				close(0);
				close(memfd);
				close(swapfd);
				close(devfd);
				dup(fdes[0]);
				close(fdes[0]);
				close(fdes[1]);
				execl(sort, &sort[SORT], 0);
				printf(cant, sort);
				exit(1);
			}
			else				/* parent */
			{
				close(fdes[0]);
				fout = fdes[1];
			}
		}
		else fout = dup(1);
	}
	if (col == 0)
	{
		printf("%c: %5d %5d", findtty(), pid, uid);
		col = 1;
	}
	while (col++ < newcol) printf("      ");
	printf(" %5d", inum);
	if (chkflag) addtolist(inum);
}


findtty()
{
	register m, n, p;
	int ttydev;

	if (tty == 0) return '?';
	n = 0;
	while (m = ttyp[n])
	{
		if (m == tty.unsd) return ttyc[ttyi[n]];
		n++;
	}
	if (n != TTYS)
	{
		seek(memfd, &tty->t_dev, 0);
		read(memfd, &ttydev, 2);
		m = 0;
		while ((p = ttyd[m]) != -1)
		{
			if (p == ttydev)
			{
				ttyp[n] = tty;
				return ttyc[ttyi[n] = m];
			}
			m++;
		}
	}
	return '!';
}


addtolist(inum)
register inum;
{
	register *lp;

	for (lp = list; lp != &list[li] && *lp != inum; lp++);
	if (lp == &list[li] && li != NI)
	{
		*lp = inum;
		li++;
	}
}


char *itoa(p, n)
char *p;
register unsigned n;
{
	register char *sp, *rp;
	char stack[6];

	rp = p;
	sp = stack; *sp++ = 0;
	while (n) { *sp++ = n % 10 + '0'; n =/ 10; }
	while (*rp++ = *--sp)
		if (rp >= &args[sizeof args]) return p;
	return rp;
}


callchk(filsys)
register char *filsys;
{
	register i;
	register char *cp;
	char *arglist[NI + 4];

	close(memfd);
	close(swapfd);
	close(devfd);
	arglist[0] = &chk[CHK];
	arglist[1] = "-i";
	arglist[(i = li) + 2] = filsys;
	arglist[i + 3] = 0;
	cp = args;
	while (i--)
	{
		arglist[i + 2] = cp;
		cp = itoa(cp, list[i]);
	}
	for (i = 0; arglist[i] != 0; printf(" %s", arglist[i++]));
	putchar('\n');
	execv(chk, arglist);
	printf(cant, chk);
	exit(1);
}
