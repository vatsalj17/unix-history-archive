#include <sys/types.h>
#include <sys/stat.h>
#include <sys/pex.h>
#include <setjmp.h>
#include <libc.h>
#include <errno.h>
#include <signal.h>
#include <cbt.h>
#include <ipc.h>

extern char *xs();

#define NAMLEN 256
#define INDLEN 8
#define PTLEN 8

struct item {
	char name[NAMLEN];
	char datum[PTLEN+INDLEN];
	mbuf user;
	mbuf val;
	int color;
};
enum {plaintext=0, ciphertext=1};

char masterkey[PTLEN];
char *db;
char *dbf;
char *odb;
int ttyfd;
int pid;
int cbtfd;				/* handle for pex */
ipcinfo *ip;
jmp_buf  env;

char *glue3();
int getitem();
char *getline();

int	xxx();
int	yyy();
int	arm();
int	disarm();
