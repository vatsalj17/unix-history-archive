#define BUFSIZE 4096
#include "fio.h"
#include "sys/label.h"
#include "sys/log.h"
extern struct label labelbot;
char *labtoa();

struct logbuf  abuf;
char *sysname[];

char *xp;
int xn;
main(ac, av)
char **av;
{
	long at;
	int i, fd;

	if(ac < 2 || ac > 3) {
		Fprint(1, "usage: logpr file [offset]\n");
		exit(0);
	}
	fd = open(av[1], 0);
	if(fd == -1) {
		perror(av[1]);
		exit(1);
	}
	Finit(fd, 0);
	if(ac > 2)
		Fseek(fd, atol(av[2]), 0);
	Fprint(1,"offset: [ seqno: pid/code/mode: ... ]\n");
	for(;;) {
		at = FIOSEEK(fd);
		Fprint(1,"%d: ", at);
		if(Fread(fd, (char*)&abuf, 11) < 11) break;
		if(abuf.len<11 || abuf.len>sizeof(abuf) || abuf.colon!=':') {
			Fprint(1, "\n");
			Fseek(fd, at+1, 0);
			continue;
		}
		xp = abuf.body;
		xn = abuf.len-11;
		if(Fread(fd, xp, xn) < xn) break;
		
		Fprint(1,"[ %ld: %d/%o/%d:", 
			abuf.slug, abuf.pid, 0xff&abuf.code, 0xff&abuf.mode);
		
		switch(abuf.code) {
		default:
			Fprint(1," %d unknown\n", abuf.code);
			break;
		
		case  LOG_USER:
			Fprint(1,"    user record:");
			while((i=BYTE()) >= 0) Fputc(1, i);
			break;
		case  LOG_TM:
			Fprint(1,"    TM use");
			break;
		case  LOG_SYSCALL:
			i = abuf.mode;
			Fprint(1,"    %ssyscall %s", 
				(i&0200)?"bad ":"", sysname[0177&i]);
			break;
		case  LOG_NAMEI:	
			Fprint(1,"    namei (%d)", abuf.mode);
			break;
		case  LOG_TU:
			Fprint(1,"    TU use"); 
			break;
		case  LOG_EXEC:
			Fprint(1,"    %sEXEC w/ %d args", 
				abuf.mode?"":"LEVEL DROP ", abuf.mode);
			break;
		case  LOG_PLAB:
			Fprint(1,"    PLAB "); 
			break;
		case  LOG_SLAB:
			Fprint(1,"    SLAB "); 
			break;
		case  LOG_LOGNAME:
			Fprint(1,"    LOGNAME"); 
			break;
		case LOG_EXTERN:
			Fprint(1,"   exercise MOUNT %d", abuf.mode);
			break;
		case LOG_NOCHK:
			Fprint(1,"   exercise NOCHK %d", abuf.mode);
			break;
		case LOG_SETLIC:
			Fprint(1,"   exercise SETLIC %d", abuf.mode);
			break;
		case LOG_SPRIV:
			Fprint(1,"   exercise SETPRIV %d", abuf.mode);
			break;
		case LOG_UAREA:
			Fprint(1,"   exercise UAREA %d", abuf.mode);
			break;
		case LOG_OPEN:
			Fprint(1,"   +fd %d", abuf.mode);
			break;
		case LOG_CLOSE:
			Fprint(1,"   -fd %d", abuf.mode);
			break;
		case LOG_FMOUNT:
			Fprint(1,"   fmount %d", abuf.mode);
			break;
		case LOG_LOG:
			Fprint(1,"   log %d", abuf.mode);
			break;
		case LOG_SEEK:
			Fprint(1,"   seek %d", abuf.mode);
			break;
		case LOG_FLAB:
			Fprint(1,"   float lab %d", abuf.mode);
			break;
		case LOG_TRACE:
			Fprint(1,"   poison file trace %d", abuf.mode);
			break;
		case LOG_EXIT:
			Fprint(1,"   exit rv=%d", abuf.mode);
			break;
		}
		while(parse());
		Fprint(1,"]\n");
		/* Fflush(1); ? */
	}
	exit(0);
}
BYTE() {
	if(xn-- < 1) return -1;
	else return *xp++ & 0xff;
}
number(k) {
	int s, x;
	s = x = 0;
	while(k--) {
		x += BYTE()<<s;
		s += 8;
	}
	if(xn >=0) return x;
	else return -1;
}
parse() {
	short *shp;
	struct label lab;
	int i, c, x;
	if(xn<1) return 0;
	switch(c = BYTE()) {
	case 'C':
	case '$':
	case 's':
		x = number(c=='s'?2:1);
		Fprint(1," %c<", c);
		while(x--) {
			c = BYTE();
			if(c == -1) break;
			Fputc(1, c);
		}
		Fprint(1,">");
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
		Fprint(1," <%x>", number(c-'0'));
		break;
	default:
		Fprint(1," unknown code %c = %03o",c, c);
		break;
	case 'E':
		Fprint(1," did ELAB");
		break;
	case 'I':
		Fprint(1, " inode %o", number(2));
		Fprint(1, "/%d.", number(2));
		break;
	case 'e':
		Fprint(1," error %d", BYTE());
		break;
	case 'j':
		lab = labelbot;
		shp = (short *) &lab.lb_priv;
		*shp = number(2);
		Fprint(1," jlabel <%13.13s,slabp = %d>", labtoa(&lab), number(2));
		break;
	case 'J':
		lab = labelbot;
		shp = (short *) &lab.lb_priv;
		*shp = number(2);
		Fprint(1," Jlabel slabp = %d", number(2));
		for(i=0; i<LABSIZ; i++)
			lab.lb_bits[i] = BYTE();
		Fprint(1,"  bits=<%s>", labtoa(&lab));
	}
	return 1;
}

char *sysname[] = {
"0", "rexit", "fork", "read", "write", "open", "close", "wait",
"creat", "link", "unlink", "11", "chdir", "gtime", "mknod", "chmod",
"chown", "sbreak", "stat", "seek", "getpid", "21", "dirread", "setuid",
"getuid", "stime", "fmount", "alarm", "fstat", "pause", "utime", "fchmod",
"fchown", "saccess", "nice", "ftime", "sync", "kill", "select", "setpgrp",
"lstat", "dup", "pipe", "times", "profil", "45", "setgid", "getgid",
"ssig", "49", "funmount", "sysacct", "biasclock", "syslock", "ioctl", "sysboot",
"setruid", "symlink", "readlink", "exece", "umask", "61", "62", "63",
"rmdir", "mkdir", "vfork", "getlogname", "64+4", "64+5", "64+6", "64+7",
"vadvise", "64+9", "setgroups", "getgroups", "64+12", "vlimit", "bcreat", "syslog",
"lmount", "64+17", "64+18", "64+19", "64+20", "vswapon", "64+22", "64+23",
"64+24", "64+25", "64+26", "64+27", "64+28", "nochk", "getflab", "fgetflab",
"setflab", "fsetflab", "getplab", "setplab", "unsafe", "seeknoret", "tell", "mktemp",
"insecure", "nap", "labmount", "vtimes", "64+44", "64+45", "64+46", "64+47",
"64+48", "64+49", "64+50", "64+51", "64+52", "64+53", "64+54", "64+55",
"64+56", "64+57", "64+58", "64+59", "64+60", "64+61", "64+62", "limits",
};
