/*
 * Multilevel mail reader
 * If running low, remove dead files from maildir
 * If running high, mark them for removal by truncating:
 *	when next invoked, trusted blackmail will remove them.
 */
#include <sys/label.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <ndir.h>
#include <errno.h>

#define NLET 100
#define QUIT (-1^((unsigned)-1>>1))	/* -infinity */
#define EXIT (QUIT+1)
#define NOP (QUIT+2)

extern char *strdup(), *getenv(), *malloc(), *strcpy(), *strchr();
extern char *strncpy(), *strcat();
extern void exit();
struct letter {
	int delete;
	char *name;
	char *header;
	struct stat statb;
} letter[NLET];
int n;
int iflag;
int atbottom;
extern struct label labelbot;
char *maildir;
char *mbox;
SIG_TYP oldsig;

compar(p,q)
struct letter *p, *q;
{
	return q->statb.st_mtime - p->statb.st_mtime;
}

interrupt()
{
	iflag = 1;
	signal(SIGINT, interrupt);
}

char *name(fname)
char *fname;
{
	char s[100];
	strcpy(s, maildir);
	strcat(s, fname);
	return strdup(s);
}

main(argc, argv)
char **argv;
{
	struct label lab;
	char *home = getenv("HOME");
	DIR *mailDIR;
	struct direct *dirent;
	struct stat stat1, stat2;
	char hbuf[200], *t;
	int i;

	if(argc > 1) {
		execv("/bin/mail", argv);
		perror("/bin/mail");
		exit(1);
	}
	if(home == 0) 
		error("do not know $HOME", "");
	maildir = malloc(strlen(home)+10);
	strcpy(maildir, home);
	mbox = name("/mbox");
	strcat(maildir, "/.mail/");
	if(getplab(&lab, (struct label*)0) < 0)
		error("bug. cannot getplab", "");
	if(setplab(0, &lab) <0)
		error("bug. cannot setplab", "");
	atbottom = labEQ(&lab, &labelbot);
	t = name("FLAG");
	if(atbottom)
		unlink(t);
	else {
		if(fork() == 0) {
			close(0);
			execl("/usr/bin/blackmail", "blackmail", maildir, 0);
			exit(1);
		}
		while(wait((int*)0)!=-1);
	}
	mailDIR = opendir(maildir);
	if(stat(maildir, &stat1) < 0)
		error("cannot stat ", maildir);
	if(mailDIR == 0)
		error("cannot open ", maildir);
	for(n=0; n<NLET; ) {
		FILE *f;
		char *pathname;
		
		errno = 0;
		dirent = readdir(mailDIR);
		if(dirent == 0)
			break;
		pathname = name(t = dirent->d_name);
		if(strcmp(t, "FLAG") == 0) {
			creat(pathname, 0644);
			continue;
		}
		if(strcmp(t, ".") == 0)
			continue;
		if(strcmp(t, "..") == 0)
			continue;
		letter[n].name = pathname;
		f = fopen(pathname, "r");
		if(f == NULL || fstat(fileno(f), &letter[n].statb)<0)
			perror(pathname);
		else {
			if(fgets(hbuf, sizeof(hbuf)-1, f) == NULL)
				strcpy(hbuf, "EMPTY");
			if(t=strchr(hbuf, '\n')) *t = 0;
			letter[n].header = strdup(hbuf);
			n++;
			fclose(f);
		}
	}
	if(n >= NLET)
		fprintf(stderr, "too many letters, some ignored\n");
	if(errno)
		perror(maildir);
	qsort((char*)letter, n, sizeof *letter, compar);
	oldsig = signal(SIGINT, interrupt);
	for(i=0; i<n; ) {
		char *pathname = letter[i].name;
		struct stat *sb;
		FILE *file;
		int a;

		errno = 0;
		file = fopen(pathname, "r");
		if(file == 0) {
			perror(pathname);
			i++;
			continue;
		}
		if(letter[i].statb.st_size == 0) {
			letter[i++].delete = 1;
			continue;
		}
		copyfile(file, stdout);
		for( ; ; ) {
			fflush(stdout);
			iflag = 0;
			a = query(file, i);
			if(iflag) {
				continue;
			}
			switch(a) {
			case QUIT:
				goto quit;
			case EXIT:
				return;
			case NOP:
				continue;
			}
			i = a;
			fclose(file);
			break;
		}
	}
quit:
	stat(maildir, &stat2);
	for(i=0; i<n; i++)
		if(letter[i].delete) {
			int x;
			t = letter[i].name;
			if(atbottom)
				x = unlink(t);
			else
				x = creat(t, 0644);
			if(x < 0)
			   perror(t);
		}
	if(stat1.st_mtime != stat2.st_mtime)
		fprintf(stderr, "New mail arrived.\n");
}

query(file, i)
FILE *file;
{
	FILE *o;
	int j;
	char s[200], *t, u[200];
	
	fprintf(stderr, "? ");
	if(fgets(s,sizeof(s),stdin) == 0)
		return QUIT;
	if(iflag) {
		return 0;	/* retval ignored */
	}
	t = strchr(s, '\n');
	if(t) *t = 0;
	switch(s[0]) {
	case 'p':
		return i;
	case 'u':
		letter[i].delete = 0;
		return NOP;
	case 'd':
		letter[i].delete = 1;
		return i+1;
	case 'q':
		return QUIT;
	case 'x':
		return EXIT;
	case 0:
		return i+1;
	case 'w':
		fgets(u, sizeof(u), file);
		/* fall thru */
	case 's':
		t = s+1;
		while(*t == ' ') t++;
		if(*t == 0) {
			t = mbox;
		}
		o = fopen(t, "a"); 
		if(o == 0)
			perror(t);
		else {
			fprintf(stderr, "appending to %s\n", t);
			copyfile(file, o);
			putc('\n', o);
			fflush(o);
			if(!ferror(o))
				letter[i].delete = 1;
			fclose(o);
		}
		return i+1;
	case 'm':
	case 'M':
		strcpy(u, "mail ");
		strcpy(u+5, s+1);
		if(strchr(u+4, 0) == 0) {
			fprintf(stderr, "no address\n");
			return i;
		}
		pipecopy(s[0]=='M'?stdin:0, file, u);
		letter[i].delete = 1;
		return i+1;
	case '|':
		pipecopy((FILE*)0, file, s+1);
		fprintf(stderr, "!\n");
		return i+1;
	case 'r':
	case 'R':
		strcpy(u, letter[i].header);
		strncpy(u, "mail", 4);
		*strchr(u+5, ' ') = 0;
		pipecopy(stdin, s[0]=='R'?file:0, u);
		return NOP;
	case 'h':
		hprint(i);
		return NOP;
	case 'b':
		for(j=i; j<n && j< i+10; j++)
			hprint(j);
		return NOP;
	case '=':
		fprintf(stderr, "%d\n", i+1);
		return NOP;
	case '-':
		return max(0, i-1);
	default:
		fprintf(stderr, "d	mark for deletion\n");
		fprintf(stderr, "m addr	remail message to addr\n");
		fprintf(stderr, "M addr	remail preceded by user input\n");
		fprintf(stderr, "p	print again\n");
		fprintf(stderr, "q	quit, deleting marked messages\n");
		fprintf(stderr, "x	quit, without deleting\n");
		fprintf(stderr, "r	reply to sender\n");
		fprintf(stderr, "R	reply; append orig\n");
		fprintf(stderr, "u	unmark for deletion\n");
		fprintf(stderr, "	next message\n");
		fprintf(stderr, "-	prev message\n");
		fprintf(stderr, "s file	append to file\n");
		fprintf(stderr, "w file	append to file, sans header\n");
		fprintf(stderr, "!com	execute command\n");
		fprintf(stderr, "|com	pipe message into command\n");
		fprintf(stderr, "number	go to message number\n");
		fprintf(stderr, "=	print curr message number\n");
		fprintf(stderr, "h	print curr message header\n");
		fprintf(stderr, "b	print 10 message headers\n");
		fprintf(stderr, "?	help\n");
		return NOP;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return min(n-1, max(0, atoi(s)-1));
	case '!':
		system(s+1);
		fprintf(stderr, "!\n");
		return NOP;
	}
}

hprint(i)
{
	fprintf(stderr, "%d%c: %5d %s\n", 
		i+1,
		letter[i].delete[" D"],
		letter[i].statb.st_size,
		letter[i].header);
}

copyfile(i, o)
FILE *i, *o;
{
	clearerr(i);
	clearerr(o);
	fseek(i, 0L, 0);
	for( ; ; ) {
		char buf[BUFSIZ];
		int n;
		n = fread(buf, 1, BUFSIZ, i);
		if(n<=0 || iflag)
			break;
		errno = 0;
		n = fwrite(buf, 1, n, o);
		if(n <= 0 && errno != 0)
			perror("output write error");
		if(n<=0 || iflag)
			break;
	}
	iflag = 0;
}

copytty(o)
FILE *o;
{
	int x, state = 0;

	fprintf(stderr, "[end with EOT or .]\n");

	for(;;) {
		x = getchar();
		if(iflag) {
			clearerr(stdin);
			break;
		}
		if(x == EOF) {
			if(state) putc('\n', o);
			clearerr(stdin);
			break;
		}
		if(state==0 && x=='.') {
			state = 1;
			continue;
		} else if(state == 1)
			if(x == '\n') break;
			else  putc('.', o);
		state = x=='\n'? 0: 2;
		putc(x, o);
		if(x == '\n')
			fflush(o);	/*for interrupt when piping*/
	}
	fprintf(stderr, "[EOT]\n");
}


pipecopy(i1, i2, s)
FILE* i1, *i2;
char *s;
{
	FILE *o;
	signal(SIGINT, oldsig);
	o = popen(s, "w");
	signal(SIGINT, interrupt);
	if(o == 0) {
		perror(s);
		return;
	}
	fprintf(stderr, "%s\n", s);
	if(i1 == stdin) copytty(o); else if(i1) copyfile(i1, o);
	if(i2 == stdin) copytty(o); else if(i2) copyfile(i2, o);
	pclose(o);
}

error(s, t)
char *s, *t;
{
	fprintf(stderr, "%s %s\n", s, t);
	exit(1);
}
