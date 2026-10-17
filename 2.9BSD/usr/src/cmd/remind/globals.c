#include "remind.h"

int lflag = 0;
int year, month, day, hour, minute;
char *loginfile = "/etc/utmp";
char *usrfile	= "/etc/passwd";
char *ttname	= "/dev/";

char *rmdlock   = "/usr/spool/rmd/rmdlock";       /* exists while proc is busy */
char *reminders = "/usr/spool/rmd/rmdfile";
char *rmdproc   = "/usr/spool/rmd/rmdproc";       /* contains proc id */
char *rmdtemp   = "/usr/spool/rmd/rmdtemp";

char *dummy     = "/dev/null";
char *nofile	= "Unable to open %s\n";
char *invalid   = "Invalid argument ... %s\n";
char shell[80];
int Uid;

struct rmdblock m;
struct stat ttstat;

char	rcvrlist[MAXRCVRS][8];
char	exdir[DIRSIZE];
char	msg[MAXMSG];

int dmsize[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
