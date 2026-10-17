#include "hd.h"
#include "command.h"

extern date(), showerror(), showgrep(), file(), home(), grep(),
	wmake(), fmake(), callshell(), longlist(), remove(),
	create(), exec(), display(), options();

struct classstruct classtab[] = {
	"date",		date,		0, 1,
	"showerror",	showerror,	0, 1,
	"showgrep",	showgrep,	0, 1,
	"file",		file,		-1, 1,
	"home",		home,		0, 1,
	"grep",		grep,		0, 1,
	"wmake",	wmake,		0, 1,
	"fmake",	fmake,		0, 1,
	"exec",		exec,		-2, 1,
	"shell",	callshell,	-1, 1,
	"display",	display,	-1, 1,
	"create",	create,		0, 1,
	"longlist",	longlist,	0, 0,
	"remove",	remove,		0, 0,
	"options",	options,	-1, 1,
	"null",		0,		0, 0,
	"",		0,		0, 0
};

static char *whoargv[]  = {"/usr/ucb/w", 0};
static char *yankargv[] = {"/usr/local/yank", 0};
static char *rootargv[] = {"/", 0};
static char *helpargv[] = {"/usr/lib/vsh/genhelp", 0};
static char *psargv[] = {"/bin/ps", 0};
static char *bshargv[] = {"/bin/sh", 0};

char *cnull = 0;

struct cmdstruct cmdtab[] = {
	'C',	create,		&cnull, 1,
	'D',	date,		&cnull, 1,
	'E',	showerror,	&cnull, 1,
	'F',	file,		&cnull, 1,
	'G',	grep,		&cnull, 1,
	'I',	display,	&cnull, 1,
	'L',	longlist,	&cnull, 0,
	'M',	wmake,		&cnull, 1,
	'N',	fmake,		&cnull, 1,
	'O',	options,	&cnull, 1,
	'P',	exec,		psargv, 1,
	'R',	remove,		&cnull, 0,
	'S',	showgrep,	&cnull, 1,
	'W',	exec,		whoargv, 1,
	'Y',	exec,		yankargv, 1,
	'^',	home,		&cnull, 1,
	':',	home,		&cnull, 1,
	'!',	callshell,	&cnull, 1,
	'%',	callshell,	&cnull, 1,
	'$',	exec,		bshargv, 1,
	'/',	file,		rootargv, 1,
	'?',	display,	helpargv, 1,

	'A',0,0,0,'B',0,0,0,
	'H',0,0,0,'J',0,0,0,'K',0,0,0,
	'O',0,0,0,'Q',0,0,0,
	'T',0,0,0,'U',0,0,0,'V',0,0,0,'X',0,0,0,
	'Z',0,0,0,'"',0,0,0,'#',0,0,0,
	'&',0,0,0,'(',0,0,0,')',0,0,0,'\'',0,0,0,
	'*',0,0,0,'=',0,0,0,'{',0,0,0,'}',0,0,0,'[',0,0,0,']',0,0,0,
	'~',0,0,0,',',0,0,0,'.',0,0,0,

	CMD_DATE, date,		&cnull, 1,
	CMD_SE,	showerror,	&cnull, 1,
	CMD_SG,	showgrep,	&cnull, 1,
	0,0,0,0
};

struct parmstruct parmtab[] = {
	"editor", "/bin/vi",
	"make", "/bin/make",
	"grep", "/usr/ucb/grep",
	"rmhelp", "/usr/lib/vsh/rmhelp",
	"showhelp", "/usr/lib/vsh/showhelp",
	"makerror", ".makerror",
	"grepout", ".grepout",
	0, 0
};
