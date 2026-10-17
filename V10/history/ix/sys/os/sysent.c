/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected,
 * a security check rule, and a pointer to the routine.
 */

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/label.h"
#include "sys/log.h"

int	alarm();
int	biasclock();
int	chdir();
int	chmod();
int	chown();
int	chroot();
int	close();
int	creat();
int	dirread();
int	dup();
int	exece();
int	fchmod();
int	fchown();
int	fmount();
int	fork(), vfork();
int	fstat();
int	ftime();
int	funmount();
int	getgid();
int	getlogname();
int	getpid();
int	getuid();
int	gtime();
int	ioctl();
int	kill();
int	link();
int	lseek();
int	lstat();
int	mkdir();
int	mknod();
int	nap();
int	nice();
int	nosys();
int	nullsys();
int	open();
int	pause();
int	pipe();
int	profil();
int	read();
int	readlink();
int	sysboot();
int	rexit();
int	rmdir();
int	saccess();
int	sbreak();
int	seek();
int	select();
int	setgid();
int	setpgrp();
int	setuid();
int	setruid();
int	ssig();
int	stat();
int	stime();
int	symlink();
int	sync();
int	sysacct();
int	syslock();
int	times();
int	umask();
int	unlink();
int	utime();
int	wait();
int	write();

int	vadvise();		/* later, segadvise */
int	vlimit();
int	vswapon();
int	vtimes();

int	getflab();
int	fgetflab();
int	setflab();
int	fsetflab();
int	getplab();
int	setplab();
int	unsafe();
int	seeknoret();
int	tell();
int	nochk();
int	labmount();
int	syslog();
int	get5state();

struct sysent sysent[MAXSYS] = {
	{0,	0,	0,		0,	nosys},		/*  0 */
	{1,	0,	LP,		"1",	rexit},		/*  1 */
	{0,	0,	LP,		"R2",	fork},		/*  2 */
	{3,	RCHD,	0,		0,	read},		/*  3 */
	{3,	WCHD,	0,		0,	write},		/*  4 */
	{2,	0,	0,		"r1",	open},		/*  5 */
	{1,	0,	0,		0,	close},		/*  6 */
	{0,	0,	LP,		"r2",	wait},		/*  7 */
	{2,	0,	LI|LD|LI,	"r1",	creat},		/*  8 */
	{2,	WCHN,	LI,		"",	link},		/*  9 */
	{1,	0,	LI,		"",	unlink},	/* 10 */
	{4,	0,	LI,		"1.1",	lseek},		/* 11 */
	{1,	0,	0,		"",	chdir},		/* 12 */
	{0,	0,	0,		"",	gtime},		/* 13 */
	{3,	WCHN,	LI,		"",	mknod},		/* 14 */
	{2,	WCHN,	LI,		"",	chmod},		/* 15 */
	{3,	WCHN,	LI,		"",	chown},		/* 16 */
	{1,	0,	0,		"",	sbreak},	/* 17 */
	{2,	RCHN,	LI,		"",	stat},		/* 18 */
	{3,	0,	LI,		"1.1",	seek},		/* 19 */
	{0,	0,	0,		"",	getpid},	/* 20 */
	{0,	0,	0,		0,	nosys},		/* 21 */
	{3,	RCHD,	0,		0,	dirread},	/* 22 */
	{1,	TU,	LU,		"",	setuid},	/* 23 */
	{0,	0,	LU,		"",	getuid},	/* 24 */
	{1,	0,	0,		"",	stime},		/* 25 */
	{5,	0,	LI,		"C11..",fmount},	/* 26 */
	{1,	0,	0,		"",	alarm},		/* 27 */
	{2,	RCH,	LI,		"1",	fstat},		/* 28 */
	{0,	0,	0,		"",	pause},		/* 29 */
	{2,	WCHN,	LI,		"",	utime},		/* 30 */
	{2,	WCH,	LI,		"1",	fchmod},	/* 31 */
	{3,	WCH,	LI,		"1",	fchown},	/* 32 */
	{2,	0,	0,		"",	saccess},	/* 33 */
	{1,	0,	0,		"",	nice},		/* 34 */
	{1,	0,	0,		"",	ftime},		/* 35 */
	{0,	0,	0,		"",	sync},		/* 36 */
	{2,	0,	LP,		"2",	kill},		/* 37 */
	{4,	0,	LI,		0,	select},	/* 38 */
	{2,	0,	LU,		"",	setpgrp},	/* 39 */
	{2,	RCHN,	LI,		"",	lstat},		/* 40 */
	{2,	0,	LD,		"11r1",	dup},		/* 41 */
	{0,	0,	LD,		"R1",	pipe},		/* 42 */
	{1,	0,	0,		"",	times},		/* 43 */
	{4,	0,	0,		"",	profil},	/* 44 */
	{0,	0,	0,		0,	nosys},		/* 45 */
	{1,	TU,	LU,		"",	setgid},	/* 46 */
	{0,	0,	LU,		"",	getgid},	/* 47 */
	{2,	0,	0,		"",	ssig},		/* 48 */
	{0,	0,	0,		0,	nosys},		/* 49 */
	{1,	0,	LI,		"",	funmount},	/* 50 */
	{1,	0,	0,		"",	sysacct},	/* 51 */
	{1,	0,	0,		"",	biasclock},	/* 52 */
	{1,	0,	0,		"",	syslock},	/* 53 */
	{3,	0,	LD,		"14",	ioctl},		/* 54 */
	{1,	0,	0,		"",	sysboot},	/* 55 */
	{1,	TU,	LU,		"",	setruid},	/* 56 */
	{2,	WCHN,	LI,		"",	symlink},	/* 57 */
	{3,	RCHN,	0,		"",	readlink},	/* 58 */
	{3,	0,	LP|LL,		0,	exece},		/* 59 */
	{1,	0,	LU,		"",	umask},		/* 60 */
	{0,	0,	0,		0,	nosys},		/* 61 */
	{0,	0,	0,		0,	nosys},		/* 62 */
	{0,	0,	0,		0,	nosys},		/* 63 */
	{1,	0,	0,		"",	rmdir},		/* 64 +0 */
	{2,	0,	0,		"",	mkdir},		/* 64 +1 */
	{0,	0,	LP,		"R2",	vfork},	/* 64 +2 */
	{2,	TU,	LU,		"",	getlogname},	/* 64 +3 */
	{0,	0,	0,		0,	nosys},		/* 64 +4 */
	{0,	0,	0,		0,	nosys},		/* 64 +5 */
	{0,	0,	0,		0,	nosys},		/* 64 +6 */
	{0,	0,	0,		0,	nosys},		/* 64 +7 */
	{1,	0,	0,		"",	vadvise},	/* 64 +8 */
	{0,	0,	0,		0,	nosys},		/* 64 +9 */
	{0,	0,	0,		0,	nosys},		/* 64+10 */
	{0,	0,	0,		0,	nosys},		/* 64+11 */
	{0,	0,	0,		0,	nosys},		/* 64+12 */
	{2,	TU,	LU,		"",	vlimit},	/* 64+13 */
	{0,	0,	0,		0,	nosys},		/* 64+14 */
	{3,	0,	0,		"C",	syslog},	/* 64+15 */
	{0,	0,	0,		0,	nosys},		/* 64+16 */
	{0,	0,	0,		0,	nosys},		/* 64+17 */
	{0,	0,	0,		0,	nosys},		/* 64+18 */
	{0,	0,	0,		0,	nosys},		/* 64+19 */
	{0,	0,	0,		0,	nosys},		/* 64+20 */
	{1,	0,	0,		"",	vswapon},	/* 64+21 */
	{0,	0,	0,		0,	nosys},		/* 64+22 */
	{0,	0,	0,		0,	nosys},		/* 64+23 */
	{0,	0,	0,		0,	nosys},		/* 64+24 */
	{0,	0,	0,		0,	nosys},		/* 64+25 */
	{0,	0,	0,		0,	nosys},		/* 64+26 */
	{0,	0,	0,		0,	nosys},		/* 64+27 */
	{0,	0,	0,		0,	nosys},		/* 64+28 */
	{2,	0,	0,		"",	nochk},		/* 64+29 */
	{2,	RCHN,	LI,		"",	getflab},	/* 64+30 */
	{2,	RCH,	LI,		"1",	fgetflab},	/* 64+31 */
	{2,	0,	LL|LI,		"",	setflab},	/* 64+32 */
	{2,	0,	LL|LI,		"1",	fsetflab},	/* 64+33 */
	{2,	0,	LU,		"",	getplab},	/* 64+34 */
	{2,	0,	LU|LL|LP,	"",	setplab},	/* 64+35 */
	{3,	0,	0,		"",	unsafe},	/* 64+36 */
	{3,	0,	LI,		"1.1",	seeknoret},	/* 64+37 */
	{1,	0,	LI,		"1",	tell},		/* 64+38 */
	{0,	0,	0,		0,	nosys},		/* 64+39 */
	{0,	0,	0,		0,	nosys},		/* 64+40 */
	{1,	0,	0,		"",	nap},		/* 64+41 */
	{2,	0,	0,		"C",	labmount},	/* 64+42 */
	{2,	0,	0,		"",	vtimes},	/* 64+43 */
	{0,	0,	0,		0,	nosys},		/* 64+44 */
	{0,	0,	0,		0,	nosys},		/* 64+45 */
	{0,	0,	0,		0,	nosys},		/* 64+46 */
	{0,	0,	0,		0,	nosys},		/* 64+47 */
	{0,	0,	0,		0,	nosys},		/* 64+48 */
	{0,	0,	0,		0,	nosys},		/* 64+49 */
	{0,	0,	0,		0,	nosys},		/* 64+50 */
	{0,	0,	0,		0,	nosys},		/* 64+51 */
	{0,	0,	0,		0,	nosys},		/* 64+52 */
	{0,	0,	0,		0,	nosys},		/* 64+53 */
	{0,	0,	0,		0,	nosys},		/* 64+54 */
	{0,	0,	0,		0,	nosys},		/* 64+55 */
	{0,	0,	0,		0,	nosys},		/* 64+56 */
	{0,	0,	0,		0,	nosys},		/* 64+57 */
	{0,	0,	0,		0,	nosys},		/* 64+58 */
	{0,	0,	0,		0,	nosys},		/* 64+59 */
	{0,	0,	0,		0,	nosys},		/* 64+60 */
	{0,	0,	0,		0,	nosys},		/* 64+61 */
	{0,	0,	0,		0,	nosys},		/* 64+62 */
	{0,	0,	0,		0,	nosys},		/* 64+63 */
};
