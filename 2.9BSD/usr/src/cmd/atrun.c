/*
 * Run programs submitted by at.
 */
/*! Modified by PLWard, 10/30/80, USGS for csh and mail options */

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <time.h>
#include <sys/stat.h>

# define DIR    "/usr/spool/at"
# define PDIR  	"past"
# define LASTF  "/usr/spool/at/lasttimedone"
#define MV      "mv"

int	nowtime;
int	nowdate;
int	nowyear;

main(argc, argv)
char **argv;
{
	int tt, day, year, uniq;
	struct direct dirent;
	char file[DIRSIZ+1];
	FILE *dirf;

	chdir(DIR);
	makenowtime();
	if ((dirf = fopen(".", "r")) == NULL) {
		fprintf(stderr, "Cannot read at directory\n");
		exit(1);
	}
	while (fread((char *)&dirent, sizeof(dirent), 1, dirf) == 1) {
		if (dirent.d_ino==0)
			continue;
		strncpy(file, dirent.d_name, DIRSIZ);
		file[DIRSIZ] = '\0';
		if (sscanf(file, "%2d.%3d.%4d.%2d", &year, &day, &tt, &uniq) != 4)
			continue;
		if (nowyear < year)
			continue;
		if (nowyear==year && nowdate < day)
			continue;
		if (nowyear==year && nowdate==day && nowtime < tt)
			continue;
		run(file);
	}
	fclose(dirf);
	updatetime(nowtime);
	exit(0);
}

makenowtime()
{
	long t;
	struct tm *localtime();
	register struct tm *tp;

	time(&t);
	tp = localtime(&t);
	nowtime = tp->tm_hour*100 + tp->tm_min;
	nowdate = tp->tm_yday;
	nowyear = tp->tm_year;
}

updatetime(t)
{
	FILE *tfile;

	tfile = fopen(LASTF, "w");
	if (tfile == NULL) {
		fprintf(stderr, "can't write lastfile\n");
		exit(1);
	}
	fprintf(tfile, "%04d\n", t);
}

run(file)
char *file;
{
	struct stat stbuf;
	register pid, i;
	char sbuf[64];
	char outfile[DIRSIZ+1];
	extern ctime(),time();

	if (stat(file, &stbuf) == -1) exit(1);
	if (stbuf.st_nlink != 1) {
		fprintf(stderr, "atrun:  %s:  st_nlink != 1\n", file);
		unlink(file);
		exit(1);
	}
	setgid(stbuf.st_gid);
	setuid(stbuf.st_uid);
	strcpy(outfile,file);
	outfile[0]='M';
	if (fork()!=0)
		return;
	for (i=0; i<15; i++)
		close(i);
	sprintf(sbuf, "%s %.14s %s", MV, file, PDIR);
	if (system(sbuf)<0)
		perror("mv");
	chdir(PDIR);
	open("/dev/null",0);
	dup(creat(outfile,0666));
	if (pid = fork()) {
		if (pid == -1)
			exit(1);
		wait((int *)0);
		unlink(file); 
		exit(0);
	}
/*	nice(3); executed by chron which has nice of 20 already. PLW */
	execl("/bin/csh", "csh","-f", file, 0);
	execl("/usr/bin/csh", "csh","-f", file, 0);
	fprintf(stderr, "Can't execl shell\n");
	exit(1);
}
