#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <utmp.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <time.h>

#define gdadd(x)	m.tdeliver += x

#define ABSOLUTE 0100000
#define MAXRCVRS 50
#define MAXMSG	 1000
#define DIRSIZE  254    /* No bigger than 254 */
#define PRIORITY 01	/* bits */

struct dp_int
{	char	*unsgnd1;	/* Unsigned dp integer */
	char	*unsgnd2;
};

struct rmdblock
{	time_t	tdeliver;
	int	nobytes;
	time_t	tsent;
	char	sender[9];
	char	bits;
	char    dirsize;
	char	nrcvrs;
	int	msgbytes;
};

extern int lflag;
extern int year, month, day, hour, minute;
extern char *loginfile;
extern char *usrfile;
extern char *ttname;

extern char *rmdlock;       /* exists while proc is busy */
extern char *reminders;
extern char *rmdproc;       /* contains proc id */
extern char *rmdtemp;

extern char *dummy;
extern char *nofile;
extern char *invalid;
extern char shell[];
extern int Uid;

extern struct rmdblock m;

extern char	rcvrlist[MAXRCVRS][8];
extern char	exdir[];
extern char	msg[];

extern int dmsize[];
extern struct stat ttstat;
extern time_t time();
struct tm *localtime();
char *ctime();
struct passwd *getpwnam();
struct passwd *getpwuid();
