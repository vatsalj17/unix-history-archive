/* Copyright 1990, AT&T Bell Labs */
#include <stdlib.h>
#include <string.h>
#include "fsort.h"

int mflag = 0;
int cflag = 0;
int keyed = 0;

extern void readin(void);
extern void dumptotemp(void);
extern void sealstack(struct rec *p);
extern char **addarg(char*, char**, int);

FILE *input;
char *oname = "-";
char *tname[] = { "/usr/tmp"/*substitutable*/, "/usr/tmp", "/tmp", 0 };

char **files;
int nfiles;
char **option;
int noption;

main(int argc, char **argv)
{
	int n;
	files = (char**)calloc(1, sizeof(char*));
	option = (char**)calloc(1, sizeof(char*));
	for(n=1; n<argc; n++) {
		if(nfiles==0 && argv[n][0]=='-' ||
		   strncmp(argv[n], "-o", 2) == 0) {
			switch(argv[n][1]) {
			case '-':
				while(++n < argc) /* eat -o */
					files = addarg(argv[n],
						files, nfiles++);
				continue;
			case 0:
				files = addarg(argv[n], files, nfiles++);
				continue;
			case 'k':
				option = addarg(argv[n],
					option, noption++);
				if(argv[n][2])
					continue;
				if(++n<argc)
					option = addarg(argv[n],
						option, noption++);
				else
					fatal("incomplete -k","",0);
				continue;
			case 'o':
				if(argv[n][2])
					oname = argv[n]+2;
				else if(n < argc-1)
					oname = argv[++n];
				else
					fatal("incomplete -o","",0);
				continue;
			case 'T':
				if(argv[n][2])
					tname[0] = argv[n]+2;
				else if(n < argc-1)
					tname[0] = argv[++n];
				else
					fatal("incomplete -T","",0);
				continue;
			case 'y':
				optiony(argv[n]+2);
			case 'z':
				continue;
			case 't':
				if(tab)
					warn("extra -t",argv[n],0);
				if(argv[n][2])
					argv[n] += 2;
				else if(n < argc-1)
					n++;
				else
					fatal("incomplete -t","",0);
				tab = *argv[n];
				if(argv[n][1])
					warn("junk on -t",argv[n],0);
				continue;
			}
			option = addarg(argv[n], option, noption++);
		} else if(nfiles==0 && argv[n][0]=='+')
			option = addarg(argv[n], option, noption++);
		else
			files = addarg(argv[n], files, nfiles++);
	}
 	for(n=0; n<noption; ) {
		if(option[n][0]=='-' && option[n][1]=='k') {
			if(option[n][2])
				optionk(option[n]+2);
			else
				optionk(option[++n]);
			n++;
		} else 
			n += fieldarg(option[n], option[n+1]);
	}
	fieldwrapup();
	if(nfiles == 0)
		files = addarg("-", files, nfiles++);
	tabinit();
	setsigs(cleanup);

	if(cflag) {
		if(nfiles > 1)
			fatal("-c takes just one file", "", 0);
		check(files[0]);
		return 0;
	} else if(mflag) {
		merge(nfiles, 0);
		return 0;
	}
	for(n=0; n<nfiles; n++) {
		input = fileopen(files[n], "r");
		readin();
		fileclose(input, files[n]);
	}
	if(stack->head==0 && nextfile==0) {	/* empty input */
		if(strcmp(oname,"-") != 0) 
			fileclose(fileopen(oname, "w"), oname);
		return 0;
	}
	if(stack->head && stack->head->next)
		sort(stack, 0);
	if(nextfile > 0) {
		if(stack->head)
			dumptotemp();
		tabfree();
		merge(nextfile, 1);
	} else {
		FILE *f;
		f = fileopen(oname, "w");
		printout(stack->head, f, oname);
		fileclose(f, oname);
	}
	return 0;
}

char **addarg(char *arg, char **list, int n)
{
	char **result = (char**)realloc(list, sizeof(char*)*(n+2));
	result[n+1] = 0;
	result[n] = arg;
	return result;
}

void
readin(void)
{
	int n;
	struct rec *new;
	struct rec *p = stack->tail;
	struct rec *r = p? succ(p): buffer;

	for(;;) {
		if(bufmax-(uchar*)r < MINREC) {
			sealstack(p);
			dumptotemp();
			p = 0;
			r = buffer;
		}
		r->next = (struct rec*)bufmax;
		new = getline(r, input);
	recenter:
		if(new == 0) {
			r->next = 0;
			if(p)
				p->next = r;
			p = r;
			r = succ(r);
		} else if(new == ENDFILE) {
			sealstack(p);
			return;
		} else {
			sealstack(p);
			dumptotemp();
			p = 0;
			r = buffer;
			n = data(new)-(uchar*)new+new->dlen+new->klen;
			if((uchar*)r+n > bufmax)
				fatal("monster record", "", 0);
			memmove(r, new, n);
			free(new);
			new = 0;
			goto recenter;
		}
	}
}

void
sealstack(struct rec *p)
{
	if(p == 0)
		return;
	p->next = 0;
	if(stack->head == 0)
		stack->head = buffer;
	stack->tail = p;
}

void
printout(struct rec *r, FILE *f, char *name)
{
	int c, n;
	uchar *dp, *ep;
	for( ; r; r=r->next) {
		dp = data(r);
		n = r->dlen;
		ep = dp + n++;
		c = *ep;
		*ep = '\n';
		if(fwrite((char*)dp, 1, n, f) != n)
			fatal("error writing", name, 0);
		*ep = c;
	}
}

void
dumptotemp()
{
	char *tempfile = filename(nextfile++);
	FILE *temp = fileopen(tempfile,"w");

	if(stack->head == 0)
		fatal("monster record", "", 0);
	stack->tail->next = 0;		/* for good measure */
	sort(stack, 0);
	printout(stack->head, temp, tempfile);
	fileclose(temp, tempfile);
	free(tempfile);
	stack->head = stack->tail = 0;
	return;
}
