#include <sys/param.h>
#include <sys/stat.h>
#include <sys/label.h>
#include <sys/pex.h>
#include <errno.h>

extern struct label labMAX();
char *mkoutname(), *memset();
int infd = -1;
int outfd = -1;
int finis(), catch();
extern struct label labelno;

/* copy a file, preserving as many properties as possible */ 

/* needs superuser to copy somebody else's
   file preserving ownership */
/* needs T_SETPRIV to preserve privilege*/

main(argc,argv)
char **argv;
{
	char *inname;
	char *outname;
	struct stat instat;
	struct stat outstat;
	struct label inlabel;
	struct label outlabel;
	struct label plab;
	char buf[BUFSIZE];
	int n;

	if(argc != 3) {
		fprint(2,"usage: pcopy file where\n");
		exit(1);
	}
	inname = argv[1];
	outname = argv[2];
	getplab(&plab, (struct label*)0);

	if(stat(outname, &outstat) != -1)
		if((outstat.st_mode&S_IFMT)==S_IFDIR) 
			outname = mkoutname(inname, outname);
	signal(SIGINT, catch);
	signal(SIGQUIT, catch);

	if(getflab(outname, &outlabel) == -1
	&& errno != ENOENT) {
		perror(outname);
		finis(1);
	}
	if(outlabel.lb_t || outlabel.lb_u) {
		inlabel = outlabel;	/* inlabel is a temp */
		inlabel.lb_t = inlabel.lb_u = 0;
		if(setflab(outname, &inlabel) == -1) {
			perror(outname);
			fprint(2,"can't remove privilege\n");
		}
	}
	if((outstat.st_mode&S_ICCTYP) != 0
	     && chmod(outname, 0666) == -1) {
		perror(outname);
		fprint(2,"can't clear 07000 mode bits\n");
		finis(1);
	}
	
	if((infd = open(inname, 0)) == -1
	|| pex(infd, 15, (struct pexclude*)0) == -1
	|| fstat(infd, &instat) == -1) {
		perror(inname);
		finis(1);
	}

	memset((char*)&outstat,0,sizeof outstat);
	if(stat(outname, &outstat) != -1
	&& (outstat.st_mode&S_ICCTYP))	/*suid, sgid, append only*/
		chmod(outname, (int)outstat.st_mode&~S_ICCTYP);

	if((outfd = creat(outname, 0666)) == -1
	&& chmod(outname, 0666) == -1
	&& chown(outname, getuid(), getgid()) == -1) {
		perror(outname);
		finis(1);
	}
	if((outfd = creat(outname, 0666)) == -1
	|| pex(outfd, 15, (struct pexclude*)0) == -1
	|| fstat(outfd, &outstat) == -1
	|| creat(outname, 0) == -1) {	/*cuz creat/pex not atomic*/
		perror(outname);
		if(outstat.st_mode)
			fchmod(outfd, (int)outstat.st_mode); /*restore*/
		fsetflab(outfd, &outlabel);	
		finis(1);
	}
	if(outstat.st_mode&S_ICCTYP) {
		perror(outname);
		fprint(2, "unusable mode\n");
		fsetflab(outfd, &outlabel);
		finis(1);
	}

	while(n = read(infd, buf, BUFSIZE)) {
		if(n == -1) {
			perror(inname);
			finis(1);
		}
		if(write(outfd, buf, n) != n) {
			perror(outname);
			finis(1);
		}
	}
	if(fchmod(outfd, (int)instat.st_mode) == -1) {
		perror(outname);
		fprint(2,"can't set mode\n");
	}
	if(fgetflab(infd, &inlabel) == -1) {
		perror(inname);
		finis(1);
	}
	if(utime(outname,&instat.st_atime) == -1) {
		perror(outname);
		fprint(2, "can't copy file times\n");
	}
	if((outstat.st_uid!=instat.st_uid ||
	    outstat.st_gid!=instat.st_gid)
	&& fchown(outfd, instat.st_uid, instat.st_gid) == -1) {
		perror(outname);
		fprint(2, "can't set uid,gid\n");
	}
	if(fgetflab(outfd, &outlabel) == -1) {
		perror(outname);
		fprint(2, "can't get label\n");
		finis(1);
	}
	if(!labLE(&outlabel, &inlabel) && (plab.lb_u&T_EXTERN)
	&& fsetflab(outfd, &labelno) == -1
	|| fsetflab(outfd, &inlabel) == -1) {
		perror(outname);
		fprint(2, "can't set label\n");
	}

	finis(0);
}				
	
char *mkoutname(inname, dirname)
char *inname, *dirname;
{
	extern char *malloc(), *strcpy(), *strcat(), *strrchr();
	int outnamelen;
	char *outname;
	char *basename = strrchr(inname,'/');
	basename = basename? basename+1: inname;
	outnamelen = strlen(dirname)+strlen(basename)+2;
	outname = malloc(outnamelen);
	strcpy(outname,dirname);
	strcat(outname,"/");
	strcat(outname,basename);
	return outname;
}

finis(x)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	if(unpex(infd, -1) || unpex(outfd, 15)) {
		fprint(2,"can't unpex\n");
		x = 1;
	}
	exit(x);
}

catch()
{
	finis(1);
}
