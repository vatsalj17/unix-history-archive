/*
 *      MAC-32 (down)loader
 */

char    Usage[]  = "Usage: 32ld [-p] [-z] objectfile";
char	*name;

#include <sys/ttyio.h>
#include "a.out.h"
#include "aouthdr.h"
#include "filehdr.h"
#include "scnhdr.h"
#include <stdio.h>
#include <errno.h>
#include <jioctl.h>
#include "proto.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/label.h>
#include <sys/stream.h>
#include <sys/pex.h>

#define MAXRETRIES      10
#define DATASIZE        512

#define NSECTS  12

#define MPX_VER 0x5620
#define FBOMAGIC 0560
#define SENDTERMID "\033[c"
#define TERM_1_0 "\033[?8;7;1c"
#define TERMB_1_0 "\033[?8;7;2c"
#define TERM_DMD "\033[?8;"
#define TERMIDSIZE 9

long getlong();
struct filehdr fileheader;
struct aouthdr aoutheader;
struct scnhdr secthdrs[NSECTS];

#define ioctl_t struct sgttyb *
struct sgttyb   ttysave,        /* save the state of tty */
		ttyraw;
struct ttydevb ttydsave, ttydraw;
int     obj;            /* File descriptor for object file */
int     mpx;            /* Running under mpx */
long    location;
char    *file;
char    m32ld[] = "/usr/jerq/bin/32reloc";
int     nargchars;      /* Number of characters, including nulls, in args */
int     psflag;         /* Print error detection statistics */
int   maxpktdsize;
int     bootcode = JBOOT;
int     booted;
int     retries;
char *strcpy(), *strcat();
int     open();
char    *malloc();
void	exit();
long	lseek();
unsigned	alarm();

char    Load_str[] = "\033[0;0v";   /* default download (binary, standalone)*/

short speeds[16]={
	 1,     5,      7,      10,     13,     15,     20,     30,
	60,     120,    180,    240,    480,    960,    1920,   1
};

int sizes[16]={
	 16,    16,     16,     16,     16,     16,     16,     16,
	 16,    32,     32,     56,     56,     120,    60,     16
};


void    APsend();
void    Precv();
void	Write();
void pinit ();
void pstats ();
void precv();

extern int      errno;


main(argc, argv)
int argc;
register char *argv[];
{
	struct stat statbuf;
	char    *dwnldflag;

			/* Start out by checking that download is going */
			/* to a DMD with at least 1.1 firmware (not 1.0) */
	char termid[TERMIDSIZE+1];
	int lpindex;
	int count;
	struct label lab, ceil;

	for (lpindex=0; lpindex<=TERMIDSIZE; lpindex++)
		termid[lpindex] = 0;

	(void)ioctl(1, TIOCGETP, &ttysave); /* get the current state */
	if (ioctl(1, TIOCGDEV, (ioctl_t) &ttydsave) < 0) {
		ttydsave.ospeed = ttysave.sg_ospeed;
		ttydsave.ispeed = ttysave.sg_ispeed;
	}

	name = *argv;

	while(argc>1 && argv[1][0]=='-'){
		switch(argv[1][1]){
		case 'p':
			psflag++;
			break;
		case 'z':
			bootcode = JZOMBOOT;
			break;
		case '\0':
			break;
		default:
			error(0, Usage, "");
			return 1;
		}
		argv++; argc--;
	}
	if(argc<2){
		error(0, Usage, "");
		return 2;
	}

	/*
	 * see what type of download is expected
	 */
	Load_str[4] = '0';


	ttyraw = ttysave;
	ttydraw = ttydsave;
	ttyraw.sg_flags |= RAW;
	ttydraw.flags |= F8BIT;
	(void)ioctl(1, TIOCSETP, &ttyraw);
	(void)ioctl(1, TIOCSDEV, (ioctl_t) &ttydraw);
	mpx = (ioctl(1, JMUX, (ioctl_t) 0) > -1);
	if ( mpx )
		Load_str[3] = '\0';
	else {

		/* make sure we've got the correct rom */
		write(1,SENDTERMID,strlen(SENDTERMID));
		count = 0;
		while(count < TERMIDSIZE){
			lpindex = read(0,&termid[count],TERMIDSIZE); 
			if(lpindex <= 0)
				error(1, "read error", "");
			count += lpindex;
		}
		if ((strcmp(termid,TERM_1_0) == 0) ||   /* equal strings */
			(strcmp(termid,TERMB_1_0) == 0))
			error(0,"Error: Firmware older than 1.1", "");
		if (strncmp(termid,TERM_DMD,strlen(TERM_DMD)) != 0)
			error(0, "Error: 32ld needs a 5620 terminal", "");
	}

	obj = open(file = argv[1], 0);
	if(obj<0)
		error(1, "cannot open '%s'", file);
	fstat(obj, &statbuf);
	if(fgetflab(obj,&lab)==-1)
		error(1, "cannot get label '%s'",file);
	if(lab.lb_t==0 && lab.lb_u==0) {
		getplab(&lab,&ceil);
		lab.lb_t &= ~T_EXTERN;
		setplab(&lab,&ceil);
	}

	(void)Read ((char*)&fileheader, sizeof(struct filehdr));
	if(fileheader.f_magic!=FBOMAGIC)        /* FBOMAGIC is 0560 */
		error(0, "'%s' is not a MAC-32 a.out", file);
	(void)Read ((char*)&aoutheader, (int)fileheader.f_opthdr);

	if (fileheader.f_nscns > NSECTS)
		error(0,"32ld: too many sections", "");
	if((aoutheader.vstamp==MPX_VER) ^ mpx)  /* MPX_VER is 0x5620 */
		error(0, mpx? "'%s' compiled stand-alone": "'%s' compiled for mpx", file);


	boot();

	if(mpx) {
		maxpktdsize = DATASIZE;
		load(argc-1, argv+1);
	} else {
		maxpktdsize = sizes[ttydsave.ospeed & 017];
		if(maxpktdsize> MAXPKTDSIZE)
			maxpktdsize = MAXPKTDSIZE;
		pinit(speeds[ttydsave.ospeed & 017], maxpktdsize, ACKON);
		load(argc-1, argv+1);
		buzz();
		(void)ioctl(0, TIOCFLUSH, (ioctl_t) 0);
	}
	(void)ioctl(1, TIOCNXCL, (ioctl_t) 0);
cleanup:
	(void)ioctl(1, TIOCSETP, &ttysave);
	(void)ioctl(1, TIOCSDEV, (ioctl_t) &ttydsave);

	if(psflag)
		pstats(stderr);
	if(mpx==0 && argc>=4 && strcmp(argv[2], "-x")==0) {
		execv(argv[3], &argv[3]);
		exit(1);
	}
	unpex(1, 15);
	return(0);
}


char *
bldargs(argc, argv)
	char *argv[];
{
	register i;
	register char *argp, *p, *q;
	for(nargchars=0, i=0; i<argc; i++)
		nargchars+=strlen(argv[i])+1;
	if((argp=malloc((unsigned)nargchars))==0)
		error(0,"can't allocate argument chars", "");
	/* this loop is probably not necessary, but it's safe */
	for(i=0, q=argp; i<argc; i++){
		p=argv[i];
		do; while(*q++ = *p++);
	}
	return argp;
}

load(argc, argv)
	char *argv[];
{
	char *argp;
	int i;
	if(mpx){
		argp=bldargs(argc, argv);
		putlong((long)argc);
		putlong((long)nargchars);
		putlong(aoutheader.tsize);
		putlong(aoutheader.dsize);
		putlong(aoutheader.bsize);
		relocate();
		Write(argp, nargchars);
	} else
		location = aoutheader.entry;
	
	for (i=0; i < fileheader.f_nscns; ++i)
		(void)Read ((char*)&secthdrs[i], sizeof(struct scnhdr));

	sendfile();
	if(mpx){
		ioctl(1, JTOOB, 0);
	} else {
		retries = 0;
		while(freepkts != NPBUFS)
			Precv();
		(void)apsend(aoutheader.entry, (char *)0, 0);
		retries = 0;
		while(freepkts != NPBUFS)
			Precv();
	}
}

error(pflag, s1, s2)
	char *s1, *s2;
{
	register int    n;
	register int    saverrno;
	char            buf[BUFSIZ];

	saverrno = errno;
	if(booted){
		if (mpx)                /* tell dmd side to give up */
			(void)ioctl(1, JTERM, (ioctl_t) 0);
		else
			(void)apsend(0L, (char *)0, 0);
	}
	(void)ioctl(1, TIOCNXCL, (ioctl_t) 0);
	(void)ioctl(1, TIOCSETP, &ttysave);
	(void)ioctl(1, TIOCSDEV, (ioctl_t) &ttydsave);
	if(pflag){
		errno=saverrno;
		perror(s2);
	}
	fprintf(stderr, "%s: ", name);
	fprintf(stderr, s1, s2);
	fprintf(stderr, "\n");
	if(psflag)
		pstats(stderr);
	exit(1);
}
int
Read(a, n)
	char *a;
{
	register i;
	i=read(obj, a, n);
	if(i<0)
		error(1, "read error on '%s'", file);
	return(i);
}
void
Write(a, n)
	char *a;
{
	if(write(1, a, n)!=n)
		error(1, "write error to jerq", "");
	if(psflag && !mpx)
		trace(a);
}
trace(a)
	char *a;
{
	register int    i;

	for(i=0; i<(PKTHDRSIZE+PKTASIZE); i++)
		fprintf(stderr, "<%o>", a[i]&0xff);
	fprintf(stderr, "\n");
}

sendfile()
{
	register int i;
	for ( i = 0; i<fileheader.f_nscns; ++i) {
		if(secthdrs[i].s_scnptr > 0)  {
			if ((secthdrs[i].s_flags & STYP_NOLOAD) ||
			    (secthdrs[i].s_flags & STYP_DSECT))
				continue;
			(void)lseek(obj, (long)secthdrs[i].s_scnptr, 0);
			sendseg(secthdrs[i].s_paddr,secthdrs[i].s_paddr+secthdrs[i].s_size);
		}
	}
}

sendseg(strloc,endloc)
long strloc;
long endloc;
{
	char buf[DATASIZE];
	register n;
	
	while((n=Read(&buf[0], xmin(maxpktdsize, endloc-strloc)))>0){
		if(mpx)
			Write(&buf[0], n);
		else
			APsend(strloc, buf, n);
		strloc+=n;
	}
}
void
APsend(addr, bufp, count)
	long addr;
	char *bufp;
	int count;
{
	retries = 0;
	while(freepkts == 0)
		Precv();
	(void)apsend(addr, bufp, count);
}
void
Precv()
{
	char c;

	(void)alarm(3);               /* sleep at least 2 seconds */
	if(read(0, &c, 1) == 1){
		(void)alarm(0);
		if(psflag)
			fprintf(stderr, "recv <%o>\n", c&0xff);
		precv(c);
	}else if(errno != EINTR )
		error(1, "read error", "");
	else if(++retries >= MAXRETRIES)
		error(0, "load protocol failed", "");
	else if(psflag)
			fprintf(stderr, "recv timeout.. retries=%d\n",retries);
}

xmin(a, b)
	long b; /* not your average min() */
{
	return(a<b? a : (int)b);
}

long
getlong() {
	long x = 0;
	char c[4];
	
	if(read(0, c, 4) != 4) return -1;
	x  = (c[0]<<24)&0xff000000;
	x |= (c[1]<<16)&0x00ff0000;
	x |= (c[2]<< 8)&0x0000ff00;
	x |= (c[3]<< 0)&0x000000ff;
	return x;
}
putlong(x) 
long x;
{
	char c[4];
	c[0] = (x>>24)&0xff;
	c[1] = (x>>16)&0xff;
	c[2] = (x>>8)&0xff;
	c[3] = (x)&0xff;
	Write(c,4);
}

relocate(){
	int status;
	char buf[200];
	char *mktemp();
	register i;
	char	*tmpname;	/* name of temporary file for reloc */

	location = getlong();
	(void)ioctl(1, TIOCEXCL, (ioctl_t) 0);	/* must be here so PUSHLD can work in mux */
	if(location==0)
		error(0, "no memory in terminal", "");

	tmpname=mktemp("/tmp/32XXXXXX");
	(void)close(creat(tmpname, 0600));
	(void)close(obj);
	obj = open(tmpname, 2);
	if(obj<0)	/* HELP!! */
		error(1, "tmp file vanished! (%s)", tmpname);
	unlink(tmpname);
	

	switch(fork()) {
	case 0:
		i = open("/dev/null", 2);
		if(i != 0)dup2(i, 0);
		if(i != 3)dup2(i, 3);
		dup2(obj, 1);
		sprintf(buf, "%lx", location);
		/* 32reloc does fstat on file, so cannot pipe INTO it */
		execl(m32ld, m32ld, "-b", buf, file, (char*)0);
		error(1, "exec %s", m32ld);
		break;
	case -1:
		error(1, "fork %s", m32ld);
		break;
	default:
		status = 0;
		wait(&status);
		if(status)
			error(0, "reloc errors", "");
		break;
	}
	(void)lseek(obj, 0L, 0);
	(void)Read ((char*)&fileheader, sizeof(struct filehdr));
	(void)Read ((char*)&aoutheader, (int)fileheader.f_opthdr);
}

boot(){
	char c = 0;

/*
	if(ioctl(1, FIOPX, (ioctl_t) 0)!=0) {
		(void)ioctl(1, FIONPX, (ioctl_t) 0);
	}
*/
if(pex(1, 15, (struct pexclude *)0)!=0)
unpex(1, -1);
	if(mpx){
		(void)ioctl(0, TIOCFLUSH, (ioctl_t) 0);  /* toss type-ahead! */
		(void)ioctl(1, bootcode, 0);
	}else{
		write(1, Load_str,6);   /* esc sequence for download */
		while(c != 'a' && c != 1)
			(void)read(0, &c, 1); /* wait for terminal to be ready */
	}
	booted++;
}

buzz(){
	sleep(2);       /* sleep(1) not necessarily long enough */
}
