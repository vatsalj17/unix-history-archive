/*
 * Unix/v7m gracefull system shutdown - shutdown.c
 * Fred Canter 9/25/81
 *
 * Thanks to Jerry Brenner for 90% of this program.
 */
#include <stdio.h>
#include <signal.h>
#include <a.out.h>
#include <core.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/tty.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <utmp.h>

struct nlist nl[] = {
	{ "_proc" },
	{ "_swapdev" },
	{ "_nproc" },
	{ "_swplo" },
	{ "" },
};

struct	proc mproc;
struct	utmp	utmp[50];

struct	user u;
int	chkpid;
int	retcode=1;
char	*tptr;
long	lseek();
char	*strcat();
char	*strcpy();
char	*strcmp();
char	*strncmp();
char	*ttynam;
char	*ttyname();
int	mem;
int	swmem;
int	swap;
daddr_t	swplo;
int	nproc;

int	ndev;
struct devl {
	char	dname[DIRSIZ];
	dev_t	dev;
} devl[256];

char	*coref;
int	deltim;
char	umntmsg[]	= {"/etc/unmount"};
char	cnsle[]	= {"/dev/console"};
char	lokfil[]  = {"/etc/sdloglock"};

main(argc, argv)
char **argv;
{
	int	stop();
	int i;
	char *ap, c;
	int uid, puid, pid, pgid;
	int incnt;
	char inbuf[80];
	extern int deltim;

	signal(SIGINT, stop);
	chdir ("/etc");
	ttynam = ttyname(0);
	if (strcmp(cnsle, ttynam))
	{
		printf("\nShutdown can only be run from the console device\n");
		exit();
	}
	if (argc>1)
	{
		ap = argv[1];
		deltim = atoi(ap);
	}
	else
	{
		printf("\nHow many minutes until shutdown [1-99] ? ");
		for (incnt = 0; c != '\n' ; ++incnt)
		{
			c = getchar();
			inbuf[incnt] = c;
		}
		deltim = atoi(inbuf);
	}
	if (deltim > 99 || deltim < 0) {
		printf("\nShutdown time out of range\n");
		exit(0);
		}
	creat(&lokfil, 0);
	for (;deltim >= 0; --deltim)
	{
		sndall();
		if (deltim > 0)
			sleep(60);
	}
	pid = getpid();
	if(chdir("/dev") < 0) {
		fprintf(stderr, "Can't change to /dev\n");
		exit(1);
	}
	nlist(argc>2? argv[2]:"/unix", nl);
	if (nl[0].n_type==0) {
		fprintf(stderr, "No namelist\n");
		exit(1);
	}
	coref = "/dev/mem";
	if ((mem = open(coref, 0)) < 0) {
		fprintf(stderr, "No mem\n");
		exit(1);
	}
	swmem = open(coref, 0);
	/*
	 * read mem to find swap dev.
	 */
	lseek(mem, (long)nl[1].n_value, 0);
	read(mem, (char *)&nl[1].n_value, sizeof(nl[1].n_value));
	/*
	 * Find base of swap
	 */
	lseek(mem, (long)nl[2].n_value, 0);
	read(mem, (char *)&swplo, sizeof(swplo));
	/*
	 * Find number of processes allowed - NPROC
	 */
	lseek(mem, (long)nl[3].n_value, 0);
	read(mem, (char *)&nproc, sizeof(nproc));
	/*
	 * Locate proc table
	 */
	lseek(mem, (long)nl[0].n_value, 0);
	getdev();
	uid = getuid();
	printf("\nKill Process Phase\n\n");
	while(mproc.p_pid != pid)
	{
		read(mem, (char *)&mproc, sizeof mproc);
	}
	pgid = mproc.p_pgrp;
	lseek(mem, (long)nl[0].n_value, 0);
	for (i=0; i<nproc; i++)
	{
		read(mem, (char *)&mproc, sizeof mproc);
		if (mproc.p_stat==0)
			continue;
		if (mproc.p_pgrp==0 && mproc.p_uid==0)
			continue;
		puid = mproc.p_pid;
		if(mproc.p_pgrp==pgid || mproc.p_pgrp==0)
			continue;
		kill (puid, 9);
	}
	printf("******   WAIT FOR SECOND # CHARACTER   ******\n");
	printf("****** DO A SYNC, THEN HALT THE SYSTEM ******\n");
	sync();
	kill(1, 2);
	sync();
}

getdev()
{
#include <sys/stat.h>
	register FILE *df;
	struct stat sbuf;
	struct direct dbuf;

	if ((df = fopen("/dev", "r")) == NULL) {
		fprintf(stderr, "Can't open /dev\n");
		exit(1);
	}
	ndev = 0;
	while (fread((char *)&dbuf, sizeof(dbuf), 1, df) == 1) {
		if(dbuf.d_ino == 0)
			continue;
		if(stat(dbuf.d_name, &sbuf) < 0)
			continue;
		if ((sbuf.st_mode&S_IFMT) != S_IFCHR)
			continue;
		strcpy(devl[ndev].dname, dbuf.d_name);
		devl[ndev].dev = sbuf.st_rdev;
		ndev++;
	}
	fclose(df);
	if ((swap = open("/dev/swap", 0)) < 0) {
		fprintf(stderr, "Can't open /dev/swap\n");
		exit(1);
	}
}



within(adr,lbd,ubd)
char *adr;
long lbd, ubd;
{
	return((unsigned)adr>=lbd && (unsigned)adr<ubd);
}
sndall()
{
	register i;
	register struct utmp *p;
	FILE *f;

	if((f = fopen("/etc/utmp", "r")) == NULL) {
		fprintf(stderr, "Cannot open /etc/utmp\n");
		exit(1);
	}
	fread((char *)utmp, sizeof(struct utmp), 50, f);
	fclose(f);
	f = stdin;
	for(i=0; i<50; i++) {
		p = &utmp[i];
		if(p->ut_name[0] == 0)
			continue;
		sleep(1);
		if((strcmp("root", p->ut_name) == 0)
			&& (strcmp("console", p->ut_line) != 0)) {
			printf("\nShutdown aborted - ");
			printf("user logged in as `root' on %s\n",p->ut_line);
			stop();
			}
		sendmes(p->ut_line);
	}
}

sendmes(tty)
char *tty;
{
	register i;
	char t[50], buf[BUFSIZ];
	FILE *f;
	extern int deltim;

	i = fork();
	if(i == -1) {
		fprintf(stderr, "Try again\n");
		return;
	}
	if(i)
		return;
	strcpy(t, "/dev/");
	strcat(t, tty);

	if((f = fopen(t, "w")) == NULL) {
		fprintf(stderr,"cannot open %s\n", t);
		exit(1);
	}
	setbuf(f, buf);
	fprintf(f, "Broadcast Message ...\n\n");
	if(deltim > 0)
	{
		fprintf(f, "System going down in %d minutes\n\n", deltim);
		fprintf(f, "Please finish up\n\n");
	}
	else
		fprintf(f, "System going down. Bye\n\n");
	exit(0);
}

/*
 * The stop function clears the /etc/sdloglock file
 * if the shutdown is aborted via the interrupt signal.
 * This reenables logins after the sghutdown abort.
 */
stop()
{
	unlink(&lokfil);
	exit(1);
}
