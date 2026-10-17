# include "stdio.h"
# include "macros.h"
# include "fatal.h"

# define QSIZE 32
# define DOWN "The printer has been down since %s"
# define FULL "The printer queue is temporarily full.\n"
# define NOMORE "No new jobs will be accepted until the queue gets smaller.\n"
# define CONTACT "Contact UNIX operations to get the printer running again.\n"

char	*tspec	"--";
char	*bin "r711";
char	*rm;
int	offset 8;
int	notify;
int	reform	1;
char	dir[64];
int	down;
char	*qfile;

main(argc,argv)
int argc;
char *argv[];
{
	register int i;
	register char *ldir;
	int j, count;
	struct statb sb;
	struct {
		int	ino;
		char	entry[14];
	} direct;
	char *tmpfile, *name;
	FILE *iop;

	Ffile = "vp";
	Fflags = FTLMSG | FTLEXIT;
	for(i=1; i<argc && argv[i][0]=='-'; i++)
		switch(argv[i][1]) {
			case 't':
				tspec = &argv[i][2];
				continue;
			case 'b':
				bin = &argv[i][2];
				continue;
			case 'n':
				notify++;
				continue;
			case 'R':
				reform = 0;
				continue;
			case 'o':
				offset = patoi(&argv[i][2]);
				continue;
			case 'r':
				rm = &argv[i][2];
				continue;
			default:
				fatal("unknown key letter (69)");
		}
	if (stat("/usr/vpd/.error",&sb) == 0) {
		printf(DOWN,ctime(&sb.i_mtime));
		down = 1;
	}
	j = xopen("/usr/vpd",0);
	count = 0;
	while (read(j, &direct, 16) == 16)
		if (direct.ino && ++count > QSIZE)
			break;
	close(j);
	if ((count >= QSIZE || down) && rm)
		unlink(rm);
	if (count >= QSIZE) {
		printf(FULL);
		printf(NOMORE);
		if (down)
			printf(CONTACT);
		execl("/etc/vpd","vpd",0);
		exit(1);
	}

	if (i==argc) fatal("nothing to do!");
	name = logname();
	ldir = logdir();
	tmpfile = mktemp("/usr/vpd/tfXXXXX");
	qfile = mktemp("/usr/vpd/qfXXXXX");
	setsig();
	iop = xfcreat(tmpfile,0444);
	fprintf(iop,": %s\n", name);
	curdir(dir);
	fprintf(iop,"vpbrk %s %s %s\n",bin,name,qfile);
	fprintf(iop,"chdir %s\n",dir);
	if ((j=open(stringf("%s/.path",ldir),0))>=0) {
		close(j);
		fprintf(iop,"= p <%s/.path\n",ldir);
	}
	else
		fprintf(iop,"= p /bin:/usr/bin:\n");
	for (; i<argc; i++)
		fprintf(iop,"%s ",argv[i]);
	fprintf(iop,"</dev/null ");
	if (reform) {
		fprintf(iop,"^ reform %s -0 ", tspec);
		if (offset>0)
			fprintf(iop,"+p%d ", offset);
		if (100 - offset)
			fprintf(iop, "+t%d ", 100 - offset);
	}
	putc('\n', iop);
	if (rm)
		fprintf(iop,"rm -f %s\n",rm);
	fprintf(iop,"cat /usr/vpd/.X %s\n",qfile);
	fprintf(iop,"echo \"\\n\\n$0  $0  $0  $0  $0  $0  $0  $0\\n\\n\"\n");
	fprintf(iop,"date&\n");
	fprintf(iop,"wait\n");
	if (notify) {
		fprintf(iop,"echo %s finished ^ write %s >/dev/null\n",qfile,name);
		fprintf(iop,"if 0$r -ne 0 then\n");
		fprintf(iop,"echo %s finished ^ mail %s >/dev/null\n",qfile,name);
		fprintf(iop,"endif\n");
	}
	fprintf(iop,"wait\n");
	fprintf(iop,"exit 0\n");
	fclose(iop);
	link(tmpfile, qfile);
	unlink(tmpfile);
	fprintf(stderr, "Queued as %s\n",qfile);
	execl("/etc/vpd","vpd",0);
	unlink(qfile);
	fatal("can't exec /etc/vpd");
}


clean_up()
{
	unlink(qfile);
}
