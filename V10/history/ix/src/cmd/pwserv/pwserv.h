#include <errno.h>
#include <ctype.h>
#include <fio.h>
#include <libc.h>
#include <ipc.h>
#include <sys/param.h>
#include <sys/filio.h>
#include <sys/ttyio.h>
#include <sys/stat.h>
#include <sys/pex.h>


struct entry {
	char *name;
	char *pw;
	char *atalla;
	struct entry *next;
};


extern char *atalla();
extern char *getstsrc();
extern log();
extern struct entry *getent();

#define iarg struct sgttyb*
