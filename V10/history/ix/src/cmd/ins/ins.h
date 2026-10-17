#include <sys/types.h>
#include <sys/label.h>
#include <sys/stat.h>
#include <sys/filio.h>

#include <errno.h>
#include <fio.h>
#include <libc.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

extern char *temp();
int nflag;
char cpbuf[4096];
struct job {
	int command;
	char *queue;
	char *source;
	char *target;
	char *mode;
	char *owner;
	char *group;
	char *label;
	char *cap;
	char *lic;
};
struct jobqueue {
	char *queue;
	char *job;
	char *xs;
	struct jobqueue *next;
};
#define J_REM	'r'
#define	J_COPY	'c'
#define J_UPDATE	'u'

extern char *xs();

/* placate lint, some */
#define log Log
extern log();
extern char logfile[];
extern char pending[];
