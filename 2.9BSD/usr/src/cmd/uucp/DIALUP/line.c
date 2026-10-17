/*
 * line: turn tty lines off or on for uucp to use
 *
 *   Usage: line ttyname on|off
 *   Exit codes for line off:
 *	  0: line wasn't on (and shouldn't be turned on when finished)
 *	1-9: old entry in /etc/ttys (to be restored later)
 *	 10: line is busy
 *	>10: error
 */
#include <stdio.h>
#include <utmp.h>
#include <signal.h>
#define UTMP	"/etc/utmp"
#define TTYS	"/etc/ttys"
#define MAXLINE 50		/* maximum line length in TTYS */

int	WasOn;			/* flag set if line was in use as dialup */
char	*rindex();

main(ac,av)
char **av;
{
	if (ac != 3 || av[2][0] < '0' || av[2][0] > '9') {
		fprintf(stderr,"Usage: line ttyname digit\n");
		exit(20);
	}
	if (av[2][0] == '0') {
		if (inuse(av[1]))
			exit(10);
		if (untty(av[1]))
			exit(12);
		exit(WasOn-'0');
	}
	if (resetline(av[1], av[2][0]))
		exit(14);
}

/*
 * Check whether anyone is logged in on devcul.
 */
inuse(devcul)
char *devcul;
{
	register fi;
	struct utmp utmpb;
	char *tmp;

	if ((tmp=rindex(devcul,'/')) != NULL)
		devcul = tmp+1;
	fi=open(UTMP,0);
	while(read(fi,&utmpb,sizeof(utmpb)) == sizeof(utmpb))
		if (strcmp(utmpb.ut_line,devcul) == 0) {
			if ( *utmpb.ut_name != 0 ) {
				fprintf(stderr,"%s is in use\n",devcul);
				return(-1);
			}
			break;
		}
	close(fi);
	return(0);
}

/*
 *  Remove devcul from TTYS file, notify init that line is
 *  no longer available.
 */
untty(devcul)
char *devcul;
{
	register fi, ret;
	char line[MAXLINE];
	char *tty;

	if ((tty=rindex(devcul,'/')) != NULL)
		tty++;
	else
		tty = devcul;
	fi = open(TTYS,2);
	if (fi < 0) {
		fprintf(stderr,"can't modify %s\n",TTYS);
		return (-1);
	}
	while (ret=getline(fi,line)) {
		if (!strcmp(line+2,tty)) {
			WasOn = line[0];
			break;
		}
	}
	if (ret == 0)
		return(WasOn = '0');
	if (WasOn != '0') {
		lseek(fi,-(long)(ret+1),1);
		write(fi,"0",1);
	}
	close(fi);
	if (WasOn != '0') {
		kill(1,1);
		sleep(2);		/* wait for init/getty to go away */
	}
	sprintf(line,"/dev/%s",tty);
	chmod(line,0666);
	return(0);
}

/*
 *  Restore line to dialup service (removed from service by untty).
 */
resetline(tty, mode)
char *tty;
char mode;
{
	int fi, ret;
	char *tmp;
	char	line[MAXLINE];

	if (mode == '0')
		return(-1);
	fi = open(TTYS,2);
	if (fi < 0) {
		fprintf(stderr,"can't modify %s\n",TTYS);
		return (-1);
	}
	while (ret=getline(fi,line)) {
		if (!strcmp(line+2,tty)) {
			lseek(fi,-(long)(ret+1),1);
			write(fi, &mode, 1);
			close(fi);
			kill(1,1);
			return(0);
		}
	}
	fprintf(stderr,"%s not in %s now\n",tty,TTYS);
	return(-1);
}

/*
 * Read a line from the ttys file.  Null terminate at the end
 * of the name.
 */
getline(fd,buf)
char *buf;
{
	char c;
	register char *p, *lim = buf+MAXLINE-1;
	register count = 0;

	p = buf;
	while (read(fd,&c,1) && c!='\n') {
		count++;
		if (p<lim)
			*p++ = c;
	}
	*p = 0;
	for (p=buf; *p; p++)
		if (*p==' ' || *p=='\t')
			break;
	*p = 0;
	return(count);
}
