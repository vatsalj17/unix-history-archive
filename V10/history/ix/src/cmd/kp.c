#include <sys/types.h>
#include <a.out.h>
#include <stab.h>
#include <stdio.h>

#define KSTART 0x80000000
#define	KMAX	(200*1024)	/* max kernel text */
#define	KPSIZE	(50*256)	/* number of buckets */

long kprof[KPSIZE];
long kprof1[KPSIZE];
int syscnt[128], syscnt1[128];
struct nlist *syms;
struct nlist *slots[KPSIZE];
slotno(pc) {
	pc = (pc-KSTART)/(KMAX/KPSIZE);
	if(pc<0)pc = 0;
	else if(pc>=KMAX) pc = KMAX-1;
	return pc;
}
char *sysname[];
int countaddr;
int kprofaddr;
char usagestr[] = "[-t secs | -c command] [-[sfx]] [kernel]";

extern char *optarg;
extern int optind;

#define Xf	1
#define	Xs	4
#define	Xx	8

main(ac, av)
char **av;
{
	char *kernel = "/unix", *cmd = 0;
	int x;
	long totslot;
	struct nlist *np;
	int sflg = 0, c, t, p, fd, i, nsyms;
	
	x = 0;
	while((c = getopt(ac, av, "sfxt:c:")) != -1) {
		switch(c) {
		case 's': x |= Xs; break;
		case 'f': x |= Xf; break;
		case 'x': x |= Xx; break;
		case 't':
			if(cmd != 0) {
		badct:
				fprintf(stderr, "at most one of -t -c\n");
				exit(1);
			}
			t = atoi(optarg);
			break;
		case 'c':
			if(t != 0) goto badct;
			cmd = optarg;
			break;
		case '?':
		usage:
			fprintf(stderr, "%s: %s\n", av[0], usagestr);
			exit(1);
		}
	}
	if(x == 0) x = 15;
	if(optind == ac-1) kernel = av[optind];
	else if(optind != ac) {
		goto usage;
	}
	nsyms = getsyms(kernel);
	
	for(np=syms; np<syms+nsyms; np++) {
		switch(np->n_type) {
		case N_SO:	
			if(x & Xf) 
				slots[slotno(np->n_value)] = np;
			break;
		case N_TEXT:
		case N_TEXT|N_EXT:
			if(x & Xx) 
				slots[slotno(np->n_value)] = np;
			break;
		}
	}
	np = 0;
	for(i=0; i<KPSIZE; i++) {
		if(slots[i] != 0) np = slots[i];
		else slots[i] = np;
	}
	for(i=0; i<KPSIZE; i++) {
		if(np = slots[i]) np->n_value = 0;
	}
	
	fd = open("/dev/kmem", 0);
	lseek(fd, kprofaddr, 0);
	read(fd, kprof, sizeof(kprof));
	if(x & Xs) {
		lseek(fd, countaddr, 0);
		read(fd, syscnt, sizeof(syscnt));
	}
	if(t>0 || cmd != 0) {
		if(t > 0) sleep(t);
		else system(cmd);
		lseek(fd, kprofaddr, 0);
		read(fd, kprof1, sizeof(kprof1));
		if(x & Xs) {
			lseek(fd, countaddr, 0);
			read(fd, syscnt1, sizeof(syscnt));
			for(i=0; i<128; i++)
				syscnt[i] = syscnt1[i] - syscnt[i];
		}
		for(i=0; i<KPSIZE; i++)
			kprof[i] = kprof1[i]-kprof[i];
	}
	close(fd);

	for(i=0; i<KPSIZE; i++) {
		totslot += kprof[i];
		if(np = slots[i])
			np->n_value += kprof[i];
	}
	if(x & (Xf|Xx)) {
		printf("%6d	%s\n", totslot, "TOTAL");
		for(i=0; i<KPSIZE; i++) {
			if(np = slots[i]) {
				if(np->n_value) {
					printf("%6d	%s\n",
						np->n_value, np->n_un.n_name);
					np->n_value = 0;
				}
			}
		}
	}
	if(x & Xs)
		for(i=0; i<128; i++)
			if(syscnt[i])
				printf("%10s\t%d\t%d\n", 
					sysname[i], i, syscnt[i]);
	exit(0);
}
char *string;
getsyms(file)
char *file;
{
	char *calloc();
	struct nlist *np;
	int k, i, nsyms, fd, ns;
	struct exec h;

	if((fd=open(file,0)) == -1) {
		perror(file);
		exit(1);
	}
	read(fd, &h, sizeof(h));
	nsyms = h.a_syms / sizeof(struct nlist);
	syms = (struct nlist *)calloc(nsyms, sizeof(struct nlist));
	lseek(fd, N_SYMOFF(h), 0);
	read(fd, syms, h.a_syms);
	read(fd, &ns, 4);
	string = calloc(ns, 1);
	read(fd, string+4, ns-4);
	close(fd);
	for(np=syms; np<syms+nsyms; np++) {
		char *t;
		
		k = np->n_un.n_strx;
		np->n_un.n_name = string+k;
		if(strcmp("_kprof", string+k) == 0)
			kprofaddr = np->n_value;
		else if(strcmp("_syscnt", string+k) == 0)
			countaddr = np->n_value;
		/*
		switch(np->n_type) {
		case N_BFUN:	t = "bfun"; break;
		case N_SO:	t = "so"; break;
		case N_SOL:	t = "sol"; break;
		case N_ESO:	t = "eso"; break;
		case N_FN:	t = "fn"; break;
		case N_EXT|N_TEXT:	t = "T"; break;
		default:	t = 0;
		}
		if(t)
			printf("%x	%s=%x	%s\n", 
				np->n_value, t, np->n_type, np->n_un.n_name);
		*/
	}
	return nsyms;
}

char *sysname[] = 
{
"", "rexit", "fork", "read", "write", "open", "close", "wait",
"creat", "link", "unlink", "", "chdir", "gtime", "mknod", "chmod",
"chown", "sbreak", "stat", "seek", "getpid", "sysmount", "dirread", "setuid",
"getuid", "stime", "fmount", "alarm", "fstat", "pause", "utime", "fchmod",
"fchown", "saccess", "nice", "ftime", "sync", "kill", "select", "setpgrp",
"lstat", "dup", "pipe", "times", "profil", "", "setgid", "getgid",
"ssig", "", "funmount", "sysacct", "biasclock", "syslock", "ioctl", "sysboot",
"setruid", "symlink", "readlink", "exece", "umask", "", "", "",
"rmdir", "mkdir", "vfork", "getlogname", "", "", "", "",
"vadvise", "", "setgroups", "getgroups", "", "vlimit", "", "",
"", "", "", "", "", "vswapon", "", "",
"", "", "", "", "", "nochk", "getflab", "fgetflab",
"setflab", "fsetflab", "getplab", "setplab", "unsafe", "seeknoret", "tell", "mktemp",
"insecure", "nap", "", "vtimes", "", "", "", "",
"", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "limits",
};
