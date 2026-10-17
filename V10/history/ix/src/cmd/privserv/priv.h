#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/label.h>
#include <sys/filio.h>
#include <sys/ttyio.h>
#include <sys/vtimes.h>
#include <sys/pex.h>
#include <fio.h>
#include <ipc.h>

#define yyerror	error
#define zzerror	error

enum classtype {	/* bits 15-13 are class, 12..0 type in class */
	F_ACT = 1<<15,
	F_PRED = 1<<14,
	F_RGHT = 1<<13,
	F_MASK = ~(F_ACT|F_PRED|F_RGHT),

	F_CEIL = 1,	/* actions */
	F_PRIV,
	F_EXEC,
	F_DAEM,
	F_EDIT,
	F_ANYS,

	F_ID,		/* predicates */
	F_SRC,
	F_PW,

	F_RE,		/* rights */
	F_LAB,
	F_PRIV2,
	F_NODE
};

struct dfa {
	int nstates;
	char *out;
	short **delta;
	int start;
	char *map;
	char *pat;		/* the string parsed */
	char *s;		/* original string passed by user */
	char *constring; 	/* same, if s has no metachars */
};

typedef struct node Node;
typedef struct func Func;
typedef struct need Need;
typedef struct qlst Qlst;

struct node {		/* node in tree of rights */
	char *nodename;
	Node *parent;
	Node *sib;
	Node *child;
	Qlst *qlist;
	Func *rchain;
	Node *nextx;
	char may;
};
extern Node *nodeall, *rootnode;

typedef int function();
struct func {
	char *name;
	enum classtype type;
	int ac;
	function *f;
	union {
		char *string;
		struct dfa	*re;
		struct label *lp;
		Node *np;
		int priv;
	} val;
	char valid;
	char **av;
	char **dav;		/* argument list with $ substitions */
	struct dfa **rav;	/* regexp recognizers */
	Func *next;
	Func *last;
	char *comment;
};
extern Func *rightroot;


struct need {
	Func *request;
	Func *price;
	Func *payback;
	Need *nexty;
};

struct qlst {
	Func *qchain;
	Qlst *nextq;
	Qlst *lastq;
};
extern Need *needy;

extern int nflag;
extern int ttyfd;

struct uenv {
	char logname[9];
	char ttysrc[SSRCSIZ+1];
	int cap;
	int uid;
	int gid;
	int pgrp;
	struct label plab;
	struct label pceil;
	int ac;
	char **av;
};
extern struct uenv uenv;

extern char *savec;	/* comments */
extern char *editnode;
extern char *privs;
extern struct dfa *dfanil;

extern struct label *myatolab();
extern struct dfa *egrepcomp();
extern Node * mknode();
extern Node * getnode();
extern Func * getright();
extern Func * getbuilt();
extern Func * func();
extern Func * funlist();
extern Need * dupreq();
extern char **dollar();
extern void error();
extern void bomb();

extern char *calloc();
extern char *malloc();
extern char *realloc();

extern char *memset();
extern char *memcpy();

extern long lseek();
extern void exit();

extern char *strdup();
extern char *strcat();
extern char *strcpy();
extern char *strrchr();
extern char *strchr();
extern char *strpbrk();

extern char *labtoa();
extern struct label *atolab();
extern char *privtoa();
extern long time();
extern char *ctime();
