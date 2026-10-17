#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <signal.h>

/* If you don't have the new magtape ioctl's, define OLDMAGTAPE */
/* #define OLDMAGTAPE */
#ifndef OLDMAGTAPE
#include <sys/mtio.h>
#endif

#define YES 1
#define NO  0
#define min(a,b) (((a) < (b))? (a): (b))

off_t   lseek();
time_t  time();
char    *mktemp();
char	*sprintf();
char    *strcpy();
char	*strcat();
daddr_t	bsrch();
#define TBLOCK		512

#define NBLOCK		20	/* maximum tape blocking */
#define BLOCKDFLT	20	/* default tape blocking */
#define	PIPEBLOCK	8	/* default pipe blocking; 4K */
#define	FILEBLOCK	2	/* default blocking for disk archives */

#define NAMSIZ	100
union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
	} dbuf;
} dblock, tbuf[NBLOCK];

struct linkbuf {
	ino_t	inum;
	dev_t	devnum;
	int	count;
	char	pathname[NAMSIZ];
	struct	linkbuf *nextp;
} *ihead;

struct stat stbuf;

int work;       /* one of: */
#define TABLE   1
#define COMPARE 2
#define EXTRACT 3
#define REPLACE 4

char    lnflag;         /* -X  link from linkdir if identical */
char   *linkdir;        /* link from this directory if identical */
int     vflag, mt, cflag, mflag, fflag, oflag, pflag, hflag;
int	term, chksum, wflag, recno, first, linkerrok;
int	freemem = 1;
int	nblock = 0;
FILE	*vfile = stdout;

daddr_t	low;
daddr_t	high;

FILE	*tfile;
char	tname[] = "/tmp/tarXXXXXX";


char	*usefile;
char	magtape[]	= "/dev/rmt1";
int	writing = 0;

char	*malloc();

main(argc, argv)
int	argc;
char	*argv[];
{
	char *cp;
	int onintr(), onquit(), onhup(), onterm();

	if (argc < 2)
		usage();

	tfile = NULL;
	usefile =  magtape;
	argv[argc] = 0;
	argv++;
	for (cp = *argv++; *cp; cp++) 
		switch(*cp) {
		case 'f':
			usefile = *argv++;
			fflag++;
			break;
		case 'c':
			cflag++;
			work = REPLACE;
			break;
		case 'o':
			oflag++;
			break;
		case 'p':
			pflag++;
			break;
		case 'u':
			mktemp(tname);
			if ((tfile = fopen(tname, "w")) == NULL) {
				fprintf(stderr, "tar: cannot create temporary file (%s)\n", tname);
				done(1);
			}
			fprintf(tfile, "!!!!!/!/!/!/!/!/!/! 000\n");
			/* FALL THROUGH */
		case 'r':
			work = REPLACE;
			break;
		case 'v':
			vflag++;
			break;
		case 'w':
			wflag++;
			break;
		case 'C':
			work = COMPARE;
			break;
		case 'X':
			linkdir = *argv++;
			lnflag++;
			work = EXTRACT;
			break;
		case 'x':
			work = EXTRACT;
			break;
		case 't':
			work = TABLE;
			break;
		case 'm':
			mflag++;
			break;
#ifdef	UCB_SYMLINKS
		case 'h':
			hflag++;
			break;
#endif
		case '-':
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '7':
			magtape[8] = *cp;
			usefile = magtape;
			break;
		case 'b':
			nblock = atoi(*argv++);
			if (nblock > NBLOCK || nblock <= 0) {
				fprintf(stderr, "tar: invalid blocksize (max %d)\n", NBLOCK);
				done(1);
			}
			break;
		case 'l':
			linkerrok++;
			break;
		default:
			fprintf(stderr, "tar: %c: unknown option\n", *cp);
			usage();
		}

	if (work == REPLACE) {
#ifdef OLDMAGTAPE
		if (nblock == 0)
			nblock = 1;
		if (nblock != 1 && !cflag) {
			fprintf(stderr, "tar: blocked tapes cannot be updated\n");
			done(1);
		}
#endif
		if (cflag && tfile != NULL) {
			usage();
			done(1);
		}
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			signal(SIGINT, onintr);
		if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
			signal(SIGHUP, onhup);
		if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
			signal(SIGQUIT, onquit);
/*
		if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
			signal(SIGTERM, onterm);
*/
		if (strcmp(usefile, "-") == 0) {
			if (cflag == 0) {
				fprintf(stderr, "tar: can only create standard output archives\n");
				done(1);
			}
			vfile = stderr;
			mt = dup(1);
			nblock = PIPEBLOCK;
		}
		else if ((mt = open(usefile, 2)) < 0) {
			if (cflag == 0 || (mt =  creat(usefile, 0666)) < 0) {
				fprintf(stderr, "tar: cannot open %s\n", usefile);
				done(1);
			}
		}
		writing = 1;
		dorep(argv);
	}
	else if (work) {
		if (strcmp(usefile, "-") == 0) {
			mt = dup(0);
			nblock = PIPEBLOCK;
		}
		else if ((mt = open(usefile, 0)) < 0) {
			fprintf(stderr, "tar: cannot open %s\n", usefile);
			done(1);
		}
		switch (work) {
		case EXTRACT:
			doxtract(argv);
			break;
		case COMPARE:
			docompare(argv);
			break;
		case TABLE:
			dotable();
			break;
		}
	}
	else
		usage();
	done(0);
}

usage()
{
#ifdef	UCB_SYMLINKS
	fprintf(stderr, "tar: usage  tar -{txruC}[cvfblmh] [tapefile] [blocksize] file1 file2...\n");
#else
	fprintf(stderr, "tar: usage  tar -{txruC}[cvfblm] [tapefile] [blocksize] file1 file2...\n");
#endif
	done(1);
}

dorep(argv)
char	*argv[];
{
	register char *cp, *cp2;
	char wdir[60];

	if (!cflag) {
		getdir();
		do {
			passtape();
			if (term)
				done(0);
			getdir();
		} while (!endtape());
		if (tfile != NULL) {
			char buf[200];

			sprintf(buf, "sort +0 -1 +1nr %s -o %s; awk '$1 != prev {print; prev=$1}' %s >%sX; mv %sX %s",
				tname, tname, tname, tname, tname, tname);
			fflush(tfile);
			system(buf);
			freopen(tname, "r", tfile);
			fstat(fileno(tfile), &stbuf);
			high = stbuf.st_size;
		}
	}

	getwdir(wdir);
	while (*argv && ! term) {
		cp2 = *argv;
		if (!strcmp(cp2, "-C") && argv[1]) {
			argv++;
			if (chdir(*argv) < 0)
				perror(*argv);
			else
				getwdir(wdir);
			argv++;
			continue;
		}
		for (cp = *argv; *cp; cp++)
			if (*cp == '/')
				cp2 = cp;
		if (cp2 != *argv) {
			*cp2 = '\0';
			if (chdir(*argv) < 0) {
				perror(*argv);
				continue;
			}
			*cp2 = '/';
			cp2++;
		}
		putfile(*argv++, cp2);
		if (chdir(wdir) < 0) {
			fprintf(stderr, "tar: cannot change back?: ");
			perror(wdir);
		}
	}
	putempty();
	putempty();
	flushtape();
	if (linkerrok == 1)
		for (; ihead != NULL; ihead = ihead->nextp)
			if (ihead->count != 0)
				fprintf(stderr, "tar: missing links to %s\n", ihead->pathname);
}

endtape()
{
	if (dblock.dbuf.name[0] == '\0') {
		if (writing)
			backtape();
		return(1);
	}
	else
		return(0);
}

getdir()
{
	register struct stat *sp;
	int i;

	readtape( (char *) &dblock);
	if (dblock.dbuf.name[0] == '\0')
		return;
	sp = &stbuf;
	sscanf(dblock.dbuf.mode, "%o", &i);
	sp->st_mode = i;
	sscanf(dblock.dbuf.uid, "%o", &i);
	sp->st_uid = i;
	sscanf(dblock.dbuf.gid, "%o", &i);
	sp->st_gid = i;
	sscanf(dblock.dbuf.size, "%lo", &sp->st_size);
	sscanf(dblock.dbuf.mtime, "%lo", &sp->st_mtime);
	sscanf(dblock.dbuf.chksum, "%o", &chksum);
	if (chksum != checksum()) {
		fprintf(stderr, "tar: directory checksum error\n");
		done(2);
	}
	if (tfile != NULL)
		fprintf(tfile, "%s %s\n", dblock.dbuf.name, dblock.dbuf.mtime);
}

passtape()
{
	long blocks;

	if (dblock.dbuf.linkflag == '1')
		return;
	blocks = stbuf.st_size;
	blocks += TBLOCK-1;
	blocks /= TBLOCK;

	while (blocks-- > 0)
		readtape((char *)NULL);
}

putfile(longname, shortname)
char *longname;
char *shortname;
{
	int infile;
	long blocks;
	char buf[TBLOCK];
	register char *cp, *cp2;
	struct direct dbuf;
	int i, j;

	infile = open(shortname, 0);
	if (infile < 0) {
		fprintf(stderr, "tar: %s: cannot open file\n", longname);
		return;
	}

#ifdef	UCB_SYMLINKS
	if (!hflag)
		lstat(shortname, &stbuf);
	else
		stat(shortname, &stbuf);
#else
	fstat(infile, &stbuf);
#endif

	if (tfile != NULL && checkupdate(longname) == 0) {
		close(infile);
		return;
	}
	if (checkw('r', longname) == 0) {
		close(infile);
		return;
	}

	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
		for (i = 0, cp = buf; *cp++ = longname[i++];);
		*--cp = '/';
		*++cp = 0  ;
		i = 0;
		if (!oflag) {
		    if( (cp - buf) >= NAMSIZ) {
			fprintf(stderr, "tar: %s: file name too long\n", longname);
			close(infile);
			return;
		    }
		    stbuf.st_size = 0;
		    tomodes(&stbuf);
		    strcpy(dblock.dbuf.name,buf);
		    sprintf(dblock.dbuf.chksum, "%6o", checksum());
		    writetape( (char *) &dblock);
		}
		if (chdir(shortname) < 0) {
			perror(shortname);
			return;
		}
		while (read(infile, (char *)&dbuf, sizeof(dbuf)) > 0 && !term) {
			if (dbuf.d_ino == 0) {
				i++;
				continue;
			}
			if (strcmp(".", dbuf.d_name) == 0 || strcmp("..", dbuf.d_name) == 0) {
				i++;
				continue;
			}
			cp2 = cp;
			for (j=0; j < DIRSIZ; j++)
				*cp2++ = dbuf.d_name[j];
			*cp2 = '\0';
			close(infile);
			putfile(buf, cp);
			infile = open(".", 0);
			i++;
			lseek(infile, (long) (sizeof(dbuf) * i), 0);
		}
		close(infile);
		chdir("..");
		return;
	}
	i = stbuf.st_mode & S_IFMT;
#ifdef	UCB_SYMLINKS
	if (i != S_IFREG && i != S_IFLNK)
#else
	if (i != S_IFREG)
#endif
	{
		fprintf(stderr, "tar: %s is not a file. Not dumped\n", longname);
		close(infile);
		return;
	}

	tomodes(&stbuf);

	cp2 = longname;
	for (cp = dblock.dbuf.name, i=0; (*cp++ = *cp2++) && i < NAMSIZ; i++);
	if (i >= NAMSIZ) {
		fprintf(stderr, "tar: %s: file name too long\n", longname);
		close(infile);
		return;
	}

#ifdef	UCB_SYMLINKS
	if ((stbuf.st_mode & S_IFMT) == S_IFLNK) {
		if (stbuf.st_size + 1 >= NAMSIZ) {
			fprintf(stderr, "tar: %s: symbolic link too long\n",
				longname);
			close(infile);
			return;
		}
		i = readlink(longname, dblock.dbuf.linkname, NAMSIZ - 1);
		if (i < 0) {
			perror("readlink");
			close(infile);
			return;
		}
		dblock.dbuf.linkname[i] = '\0';
		dblock.dbuf.linkflag = '2';
		if (vflag) {
			fprintf(vfile, "a %s ", longname);
			fprintf(vfile, "symbolic link to %s\n",
				dblock.dbuf.linkname);
		}
		sprintf(dblock.dbuf.size, "%11o", 0);
		sprintf(dblock.dbuf.chksum, "%6o", checksum());
		writetape((char *)&dblock);
		close(infile);
		return;
	}
#endif
	if (stbuf.st_nlink > 1) {
		struct linkbuf *lp;
		int found = 0;

		for (lp = ihead; lp != NULL; lp = lp->nextp) {
			if (lp->inum == stbuf.st_ino && lp->devnum == stbuf.st_dev) {
				found++;
				break;
			}
		}
		if (found) {
			strcpy(dblock.dbuf.linkname, lp->pathname);
			dblock.dbuf.linkflag = '1';
			sprintf(dblock.dbuf.chksum, "%6o", checksum());
			writetape( (char *) &dblock);
			if (vflag) {
				fprintf(vfile, "a %s ", longname);
				fprintf(vfile, "link to %s\n", lp->pathname);
			}
			lp->count--;
			close(infile);
			return;
		}
		else {
			lp = (struct linkbuf *) malloc(sizeof(*lp));
			if (lp == NULL) {
				if (freemem) {
					fprintf(stderr, "tar: out of memory.  Link information lost\n");
					freemem = 0;
				}
			}
			else {
				lp->nextp = ihead;
				ihead = lp;
				lp->inum = stbuf.st_ino;
				lp->devnum = stbuf.st_dev;
				lp->count = stbuf.st_nlink - 1;
				strcpy(lp->pathname, longname);
			}
		}
	}

	blocks = (stbuf.st_size + (TBLOCK-1)) / TBLOCK;
	if (vflag) {
		fprintf(vfile, "a %s ", longname);
		fprintf(vfile, "%ld blocks\n", blocks);
	}
	sprintf(dblock.dbuf.chksum, "%6o", checksum());
	writetape( (char *) &dblock);

	while ((i = read(infile, buf, TBLOCK)) > 0 && blocks > 0) {
		writetape(buf);
		blocks--;
	}
	close(infile);
	if (blocks != 0 || i != 0)
		fprintf(stderr, "tar: %s: file changed size\n", longname);
	while (blocks-- >  0)
		putempty();
}



doxtract(argv)
char	*argv[];
{
	long blocks, bytes;
	char buf[TBLOCK];
	char **cp;
	int ifile;
	int ofile;
	char cmpfile[200 + NAMSIZ];
	char *fcmpfile;
	char tmpfile[200 + NAMSIZ];
	int same;

	if (lnflag) {
		strcpy (cmpfile, linkdir);
		fcmpfile = &cmpfile[strlen(cmpfile)];
		strcpy (fcmpfile, "/");
		fcmpfile++;
	}
	for (;;) {
		getdir();
		if (endtape())
			break;

		if (*argv == 0)
			goto gotit;

		for (cp = argv; *cp; cp++)
			if (prefix(*cp, dblock.dbuf.name))
				goto gotit;
		passtape();
		continue;

gotit:
		if (checkw('x', dblock.dbuf.name) == 0) {
			passtape();
			continue;
		}

		if(checkdir(dblock.dbuf.name))
			continue;

#ifdef	UCB_SYMLINKS
		if (dblock.dbuf.linkflag == '2') {
			unlink(dblock.dbuf.name);
			if (symlink(dblock.dbuf.linkname, dblock.dbuf.name)<0) {
				fprintf(stderr, "tar: %s: symbolic link failed\n",
					dblock.dbuf.name);
				continue;
			}
			if (vflag)
				fprintf(vfile, "x %s symbolic link to %s\n",
				  dblock.dbuf.name, dblock.dbuf.linkname);
			chown(dblock.dbuf.name, stbuf.st_uid, stbuf.st_gid);
			if (mflag == 0) {
				time_t timep[2];

				timep[0] = time(0);
				timep[1] = stbuf.st_mtime;
				utime(dblock.dbuf.name, timep);
			}
			if (pflag)
				chmod(dblock.dbuf.name, stbuf.st_mode & 07777);
			continue;
		}
#else
		if (dblock.dbuf.linkflag == '2') {
			fprintf(stderr, "tar: %s: symbolic links not supported\n",
				dblock.dbuf.name);
		}
#endif
		if (dblock.dbuf.linkflag == '1') {
			unlink(dblock.dbuf.name);
			if (link(dblock.dbuf.linkname, dblock.dbuf.name) < 0) {
				fprintf(stderr, "tar: %s: cannot link\n", dblock.dbuf.name);
				continue;
			}
			if (vflag)
				fprintf(vfile, "%s linked to %s\n",
					dblock.dbuf.name, dblock.dbuf.linkname);
			continue;
		}
		ifile = -1;
		same = NO;
		if (lnflag) {
			static char tmpf[] = "TarXXXXXX";
			same = YES;
			if (dblock.dbuf.name[0] != '/') {
			    strcpy (fcmpfile, dblock.dbuf.name);
			    if ((ifile = open(cmpfile, 0)) >= 0) {
				tmpfile[0] = '\0';
				strncat (tmpfile, dblock.dbuf.name,
					 dirpart (dblock.dbuf.name));
				strcat (tmpfile, tmpf);
				mktemp(tmpfile);
			    }
			}
		}
		if ((ofile = creat(ifile >= 0 ? tmpfile : dblock.dbuf.name,
				   (int) stbuf.st_mode & 07777)) < 0) {
			fprintf(stderr, "tar: %s - cannot create\n", dblock.dbuf.name);
			passtape();
			continue;
		}

		blocks = ((bytes = stbuf.st_size) + TBLOCK-1)/TBLOCK;
		if (ifile >= 0) {
			struct stat stbuf;
			if (fstat(ifile, &stbuf) < 0 || bytes != stbuf.st_size)
				same = NO;
		}
		if (vflag)
			fprintf(vfile, "x %s, %ld bytes, %ld tape blocks",
				dblock.dbuf.name, bytes, blocks);
		while (blocks-- > 0) {
			int nw;
			readtape(buf);
			nw = min(bytes, TBLOCK);
			if (write(ofile, buf, nw) < nw) {
					fprintf(stderr, "tar: %s: HELP - extract write error\n", dblock.dbuf.name);
					done(2);
				}
			if (ifile >= 0 && same)
				same = cmprd (ifile, buf, nw);
			bytes -= TBLOCK;
		}
		close(ofile);
		if (ifile >= 0) {
			close(ifile);
			unlink(dblock.dbuf.name);
			if (vflag)
				fprintf(vfile, " (%s)", same? "same" : "new");
			if (link(same? cmpfile : tmpfile, dblock.dbuf.name) < 0)
				fprintf(stderr, "tar: %s - cannot link\n",
					dblock.dbuf.name);
			unlink (tmpfile);
		}
		if (vflag)
			fprintf(vfile, "\n");

		if (ifile < 0 || !same) {
			if (mflag == 0) {
				time_t timep[2];

				timep[0] = time((time_t *) NULL);
				timep[1] = stbuf.st_mtime;
				utime(dblock.dbuf.name, timep);
			}
			chown(dblock.dbuf.name, stbuf.st_uid, stbuf.st_gid);
			if(pflag)
			    chmod(dblock.dbuf.name, (int)stbuf.st_mode & 07777);
		}
	}
}


docompare(argv)
char	*argv[];
{
	long blocks, bytes;
	char buf[TBLOCK];
	char **cp;
	struct stat stbuf2;
	int ifile;
	int same;

	for (;;) {
		getdir();
		if (endtape())
			break;

		if (*argv == 0)
			goto gotit;

		for (cp = argv; *cp; cp++)
			if (prefix(*cp, dblock.dbuf.name))
				goto gotit;
		passtape();
		continue;

gotit:
		if (dblock.dbuf.linkflag == '1') {
			if (vflag) {
				printf("Link ");
				longt(&stbuf);
				printf("%s", dblock.dbuf.name);
				printf(" linked to %s\n", dblock.dbuf.linkname);
			}
			continue;
		}
		if (dblock.dbuf.linkflag == '2') {
			if (vflag) {
				printf("Sym  ");
				longt(&stbuf);
				printf("%s", dblock.dbuf.name);
				printf(" symbolic link to %s\n", dblock.dbuf.linkname);
			}
			continue;
		}

		if ((ifile = open(dblock.dbuf.name, 0)) < 0) {
			printf("Err  ");
			if (vflag)
				longt(&stbuf);
			printf("%s: Cannot open disk file\n", dblock.dbuf.name);
			passtape();
			continue;
		}

		same = YES;
		blocks = ((bytes = stbuf.st_size) + TBLOCK-1)/TBLOCK;
		if (fstat(ifile, &stbuf2) < 0 || bytes != stbuf.st_size)
			same = NO;

		if (same)
			while (blocks-- > 0) {
				int nw;
				readtape(buf);
				nw = min(bytes, TBLOCK);
				if (same)
					same = cmprd (ifile, buf, nw);
				bytes -= TBLOCK;
			}
		else
			passtape();
		close(ifile);

		if (!same)
			printf("diff ");
		else if (vflag)
			printf("same ");
		if (vflag) {
			longt(&stbuf);
			printf("%s\n", dblock.dbuf.name);
		}
	}
}

dotable()
{
	for (;;) {
		getdir();
		if (endtape())
			break;
		if (vflag)
			longt(&stbuf);
		printf("%s", dblock.dbuf.name);
		if (dblock.dbuf.linkflag == '2')
			printf(" symbolic link to %s", dblock.dbuf.linkname);
		if (dblock.dbuf.linkflag == '1')
			printf(" linked to %s", dblock.dbuf.linkname);
		printf("\n");
		passtape();
	}
}

putempty()
{
	char buf[TBLOCK];
	char *cp;

	for (cp = buf; cp < &buf[TBLOCK]; )
		*cp++ = '\0';
	writetape(buf);
}

longt(st)
register struct stat *st;
{
	register char *cp;
	char *ctime();

	pmode(st);
	printf("%3d/%-2d", st->st_uid, st->st_gid);
	printf("%7D", st->st_size);
	cp = ctime(&st->st_mtime);
	printf(" %-12.12s %-4.4s ", cp+4, cp+20);
}

#define	SUID	04000
#define	SGID	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	STXT	01000
int	m1[] = { 1, ROWN, 'r', '-' };
int	m2[] = { 1, WOWN, 'w', '-' };
int	m3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	m4[] = { 1, RGRP, 'r', '-' };
int	m5[] = { 1, WGRP, 'w', '-' };
int	m6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	m7[] = { 1, ROTH, 'r', '-' };
int	m8[] = { 1, WOTH, 'w', '-' };
int	m9[] = { 2, STXT, 't', XOTH, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

pmode(st)
register struct stat *st;
{
	register int **mp;

	for (mp = &m[0]; mp < &m[9];)
		select(*mp++, st);
}

select(pairp, st)
int *pairp;
struct stat *st;
{
	register int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (st->st_mode&*ap++)==0)
		ap++;
	printf("%c", *ap);
}

checkdir(name)
register char *name;
{
	register char *cp;
	int i;
	for (cp = name; *cp; cp++) {
		if (*cp == '/') {
			*cp = '\0';
			if (access(name, 01) < 0) {
				register int pid, rp;

				if ((pid = fork()) == 0) {
					execl("/bin/mkdir", "mkdir", name, 0);
					execl("/usr/bin/mkdir", "mkdir", name, 0);
					fprintf(stderr, "tar: cannot find mkdir!\n");
					done(0);
				}
				while ((rp = wait(&i)) >= 0 && rp != pid)
					;
				chown(name, stbuf.st_uid, stbuf.st_gid);
				if (pflag && cp[1] == '\0')
					chmod(dblock.dbuf.name,
					  (int) stbuf.st_mode & 01777);
			}
			*cp = '/';
		}
	}
	return(cp[-1]=='/');
}

onintr()
{
	signal(SIGINT, SIG_IGN);
	term++;
}

onquit()
{
	signal(SIGQUIT, SIG_IGN);
	term++;
}

onhup()
{
	signal(SIGHUP, SIG_IGN);
	term++;
}

/*
onterm()
{
	signal(SIGTERM, SIG_IGN);
	term++;
}
*/

tomodes(sp)
register struct stat *sp;
{
	register char *cp;

	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		*cp = '\0';
	sprintf(dblock.dbuf.mode, "%6o ", sp->st_mode & 07777);
	sprintf(dblock.dbuf.uid, "%6o ", sp->st_uid);
	sprintf(dblock.dbuf.gid, "%6o ", sp->st_gid);
	sprintf(dblock.dbuf.size, "%11lo ", sp->st_size);
	sprintf(dblock.dbuf.mtime, "%11lo ", sp->st_mtime);
}

checksum()
{
	register i;
	register char *cp;

	for (cp = dblock.dbuf.chksum; cp < &dblock.dbuf.chksum[sizeof(dblock.dbuf.chksum)]; cp++)
		*cp = ' ';
	i = 0;
	for (cp = dblock.dummy; cp < &dblock.dummy[TBLOCK]; cp++)
		i += *cp;
	return(i);
}

checkw(c, name)
char *name;
{
	if (wflag) {
		printf("%c ", c);
		if (vflag)
			longt(&stbuf);
		printf("%s: ", name);
		if (response() == 'y'){
			return(1);
		}
		return(0);
	}
	return(1);
}

response()
{
	char c;

	c = getchar();
	if (c != '\n')
		while (getchar() != '\n');
	else c = 'n';
	return(c);
}

checkupdate(arg)
char	*arg;
{
	char name[100];
	long	mtime;
	daddr_t seekp;
	daddr_t	lookup();

	rewind(tfile);
	for (;;) {
		if ((seekp = lookup(arg)) < 0)
			return(1);
		fseek(tfile, seekp, 0);
		fscanf(tfile, "%s %lo", name, &mtime);
		if (stbuf.st_mtime > mtime)
			return(1);
		else
			return(0);
	}
}

done(n)
{
	unlink(tname);
	exit(n);
}

prefix(s1, s2)
register char *s1, *s2;
{
	while (*s1)
		if (*s1++ != *s2++)
			return(0);
	if (*s2)
		return(*s2 == '/');
	return(1);
}

getwdir(s)
char *s;
{
	int i;
	int	pipdes[2];

	pipe(pipdes);
	if ((i = fork()) == 0) {
		close(1);
		dup(pipdes[1]);
		execl("/bin/pwd", "pwd", 0);
		execl("/usr/bin/pwd", "pwd", 0);
		fprintf(stderr, "pwd failed!\n");
		printf("/\n");
		exit(1);
	}
	while (wait((int *)NULL) != -1)
			;
	read(pipdes[0], s, 50);
	while(*s != '\n')
		s++;
	*s = '\0';
	close(pipdes[0]);
	close(pipdes[1]);
}

#define	N	200
int	njab;
daddr_t
lookup(s)
char *s;
{
	register i;
	daddr_t a;

	for(i=0; s[i]; i++)
		if(s[i] == ' ')
			break;
	a = bsrch(s, i, low, high);
	return(a);
}

daddr_t
bsrch(s, n, l, h)
daddr_t l, h;
char *s;
{
	register i, j;
	char b[N];
	daddr_t m, m1;

	njab = 0;

loop:
	if(l >= h)
		return(-1L);
	m = l + (h-l)/2 - N/2;
	if(m < l)
		m = l;
	fseek(tfile, m, 0);
	fread(b, 1, N, tfile);
	njab++;
	for(i=0; i<N; i++) {
		if(b[i] == '\n')
			break;
		m++;
	}
	if(m >= h)
		return(-1L);
	m1 = m;
	j = i;
	for(i++; i<N; i++) {
		m1++;
		if(b[i] == '\n')
			break;
	}
	i = cmp(b+j, s, n);
	if(i < 0) {
		h = m;
		goto loop;
	}
	if(i > 0) {
		l = m1;
		goto loop;
	}
	return(m);
}

cmp(b, s, n)
char *b, *s;
{
	register i;

	if(b[0] != '\n')
		exit(2);
	for(i=0; i<n; i++) {
		if(b[i+1] > s[i])
			return(-1);
		if(b[i+1] < s[i])
			return(1);
	}
	return(b[i+1] == ' '? 0 : -1);
}

int mtdev = 1;
#ifndef	OLDMAGTAPE
struct mtget mtget;
#else
struct stat stb;
#endif

readtape(buffer)
char *buffer;
{
	register int i;
	int j;

	if (recno >= nblock || first == 0) {
#ifndef	OLDMAGTAPE
		if (mtdev == 1)
			mtdev = ioctl(mt, MTIOCGET, &mtget);
#else
		fstat(mt, &stb);
		if ((stb.st_mode & IFMT) == IFCHR)
			mtdev = 0;
		else
			mtdev = -1;
#endif
		if (first==0 && nblock==0) {
			if (mtdev == 0)
			    j = NBLOCK;
			else
			    j = FILEBLOCK;
		} else
			j = nblock;
		if ((i = read(mt, (char *) tbuf, TBLOCK*j)) < 0) {
			fprintf(stderr, "tar: tape read error\n");
			done(3);
		}
		if (first == 0) {
			if ((i % TBLOCK) != 0) {
				fprintf(stderr, "tar: tape blocksize error\n");
				done(3);
			}
			i /= TBLOCK;
#ifdef	OLDMAGTAPE
			if (work == REPLACE && i != 1) {
				fprintf(stderr, "tar: cannot update blocked tapes\n");
				done(4);
			}
#endif
			if (i != nblock ) {
				if (mtdev == 0)
				   fprintf(vfile, "tar: blocksize = %d\n", i);
				nblock = i;
			}
		}
		recno = 0;
	}
	first = 1;
	if (buffer)
		copy(buffer, (char *) &tbuf[recno]);
	recno++;
	return(TBLOCK);
}

writetape(buffer)
char *buffer;
{
	first = 1;
	if (nblock == 0)
		nblock = BLOCKDFLT;
	if (recno >= nblock) {
		if (write(mt, (char *) tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "tar: tape write error\n");
			done(2);
		}
		recno = 0;
	}
	copy((char *) &tbuf[recno++], buffer);
	if (recno >= nblock) {
		if (write(mt, (char *) tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "tar: tape write error\n");
			done(2);
		}
		recno = 0;
	}
	return(TBLOCK);
}

backtape()
{
#ifndef OLDMAGTAPE
	static struct mtop mtop = {MTBSR, 1};

	if (mtdev == 1)
		mtdev = ioctl(mt, MTIOCGET, &mtget);
	if (mtdev == 0) {
		if (ioctl(mt, MTIOCTOP, &mtop) < 0) {
			fprintf(stderr, "tar: tape backspace error\n");
			done(4);
		}
	} else
		lseek(mt, (long) -TBLOCK*nblock, 1);
	recno--;
#else
	lseek(mt, (long) -TBLOCK, 1);
	if (recno >= nblock) {
		recno = nblock - 1;
		if (read(mt, (char *) tbuf, TBLOCK*nblock) < 0) {
			fprintf(stderr, "tar: tape read error after seek\n");
			done(4);
		}
		lseek(mt, (long) -TBLOCK, 1);
	}
#endif
}

flushtape()
{
	write(mt, (char *) tbuf, TBLOCK*nblock);
}

copy(to, from)
register char *to, *from;
{
	register i;

	i = TBLOCK;
	do {
		*to++ = *from++;
	} while (--i);
}

/*  Compare the next 'nw' characters of ifile with buf.
 *   return 1 if same, else 0
 */
cmprd (ifile, buf, num)
int ifile;
char *buf;
int num;
{
	register int nr;
	char ibuf[TBLOCK];

	if (read (ifile, ibuf, num) < num)
		return NO;
	if (bufcmp (buf, ibuf, num))
		return NO;
	return YES;
}


bufcmp (cp1, cp2, num)
register char *cp1;
register char *cp2;
register int num;
{
	if (num <= 0)
		return 0;
	do
		if (*cp1++ != *cp2++)
			return *--cp2 - *--cp1;
	while (--num);
	return 0;
}

dirpart (str)
char *str;
{
	register char *cp;
	char *rindex();

	if (cp = rindex (str, '/'))
		return cp - str + 1;
	else
		return 0;
}
