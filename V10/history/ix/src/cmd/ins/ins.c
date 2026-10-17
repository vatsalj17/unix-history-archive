#include "ins.h"

char spooldir[] = "/usr/spool/ins";
char pending[] = "/usr/spool/ins/pending";
char *Strdup();
char *stash();

/*
 * flags:
 *	-v		verbose
 *	-n		report what would do, but dont do it
 *	-q qname	serialize wrt to 'qname' as well
 *
 *	-o u		chown
 *	-g g		chown
 *	-m mode		chmod
 *	-l label	setflab
 *	-r		unlink
 *	-p cap lic	setpriv
 *	-j jobname	delete job
 */

struct job args;
struct jobqueue *backlog;
int qchanged;

main(ac, av)
char **av;
{
	int i, vflag, xflag;
	char *s;

	xflag = vflag = 0;
	setarg(ac, av);
	args.command = 0;
	getstatus();
	ac--;
	av++;
	while(ac>0 && av[0][0]=='-') {
		switch(av[0][1]) {
		case 'x':
		case 'v':
		case 'n':
			for(s=av[0];*s;s++)
				if(*s=='v')vflag++;
				else if(*s=='n')nflag++;
				else if(*s=='x')xflag++;
			break;
		case 'q':
			args.queue = Strdup(*++av);
			ac--;
			break;
		case 'o':
			args.command = J_UPDATE;
			args.owner = Strdup(*++av);
			ac--;
			break;
		case 'g':
			args.command = J_UPDATE;
			args.group = Strdup(*++av);
			ac--;
			break;
		case 'm':
			args.command = J_UPDATE;
			args.mode = Strdup(*++av);
			ac--;
			break;
		case 'l':
			args.command = J_UPDATE;
			args.label = Strdup(*++av);
			ac--;
			break;
		case 'r':
			args.command = J_REM;
			break;
		case 'p':
			args.command = J_UPDATE;
			args.cap = Strdup(*++av);
			args.lic = Strdup(*++av);
			ac -= 2;
			break;
		case 'j':
			if(ac<2)
				error("ins: -j needs job numbers", "");
			if(args.command)
				error("ins: illegal -j", "");
			setlog(vflag);
			for(i=1; i<ac; i++) {
				for(s=av[i];*s; s++)
					if(!isdigit(*s))
						error("ins: bad job no. %s", av[i]);
				deljob(av[i]);
			}
			return 0;
			break;
		default:
			error("ins: unknown flag %s", *av);
			break;
		}
		ac--;
		av++;
	}
	if(xflag)verify(av[ac]);
	setlog(vflag);
	switch(ac) {
	case 0:
		if(args.command == J_REM)
			error("ins: illegal -r", "");
		if(args.command == J_UPDATE) 
			error("ins: no target file specified", "");
		return catchup((struct job *)0);
		break;
	case 1:
		if(av[0][0] != '/') error("target %s should start with /", av[0]);
		args.target = Strdup(av[0]);
		enqueue();
		return catchup(&args);
		break;
	case 2:
		if(args.command == J_REM) error("ins: illegal -r", "");
		if(av[1][0] != '/') error("target %s should start with /", av[1]);
		args.source = Strdup(av[0]);
		args.target = Strdup(av[1]);
		args.command = J_COPY;
		enqueue();
		return catchup(&args);
		break;
	default:
		error("usage: ins [args] [[source] target]","");
		return 1;
		break;
	}
	/*NOTREACHED*/
}
error(s, t)
char *s;
char *t;
{
	fprint(2, s, t);
	fprint(2, "\n");
	exit(1);
}

oldjob(s, t, xp)
char *s, *t, *xp;
{
	static struct jobqueue *last;
	struct jobqueue *p;
			
	if(s==0) return;
	p = (struct jobqueue *)calloc(1, sizeof(*p));
	p->queue = Strdup(s);
	p->job = Strdup(t);
	p->xs = Strdup(xp); 
	if(last) 
		last->next = p;
	else
		backlog = p;
	last = p;
}

getstatus() {
	char *av[10], *lp;
	int fd = open(pending, 0);

	if(fd == -1)
		return;
	while(lp=Frdline(fd)) {
		if(getmfields(lp, av, 4)==3) {
			oldjob(av[0], av[1], av[2]);
		}
	}
	close(fd);
}
#define X(s) (s)?(s):"?????????"

enqueue()
{
	struct jobqueue *j;
	char *s, *xp, *qf;
	int fd;

	if(args.command == 0 || nflag != 0)
		return;
	qf = temp("job");
	if(qf == 0)
		return;
	fd = creat(qf, 0660);
	if(fd == -1)
		return;
	ioctl(fd, FIOCLEX, (void*)0);

	for(j=backlog; j; j=j->next) {
		if(strcmp(j->queue, args.target)==0 ||
		strcmp(j->queue, args.queue)==0) {
			fprint(fd, "y %s\n", j->job);
		}
	}
	switch(args.command) {
	case J_REM:
		fprint(fd, "r %s\n", args.target);
		log("nq", "%s r %s\n", qf, args.target);
		break;
	case J_COPY:
		xp = xs(args.source, 0, 1);
		s = stash(args.source);
		fprint(fd, "c %s %s %s\n", 
			s,  args.target, xp);
		log("stash", "%s=%s -> %s %s\n", 
			s, args.source, args.target, xp);
		log("nq", "%s c %s %s %s\n", 
			qf, s,  args.target, xp);
		/* fall through... */
	case J_UPDATE:
		if(args.owner || args.group) {
			fprint(fd, "o %s %s %s\n", 
				args.target, X(args.owner), X(args.group));
			log("nq", "%s o %s %s %s\n", 
				qf, args.target, X(args.owner), X(args.group));
		}
		if(args.mode) {
			fprint(fd, "m %s %s\n", 
				args.target, args.mode);
			log("nq", "%s m %s %s\n", qf, args.target, args.mode);
		}
		if(args.label) {
			fprint(fd, "l %s %s\n", 
				args.target, args.label);
			log("nq", "%s l %s %s\n", 
				qf, args.target, args.label);
		}
		if(args.lic) {
			fprint(fd, "p %s %s %s\n", 
				args.target, args.cap, args.lic);
			log("nq", "%s p %s %s %s\n", 
				qf, args.target, args.cap, args.lic);
		}
	}
	close(fd);
	xp = xs(qf, 0, 0);
	log("jobxs", "%s %s %s %s\n", 
		qf, args.target, args.queue?args.queue: args.target, xp);
	oldjob(args.target, qf, xp);
	oldjob(args.queue, qf, xp);
	qchanged++;
}


catchup(ap)
struct job *ap;
{
	char *tag;
	struct stat sb;
	struct jobqueue *j;
	int x, fd;

	for(j=backlog; j; j=j->next)
		if(ap==0 || strcmp(X(ap->queue),j->queue) == 0
		 || strcmp(X(ap->target),j->queue) == 0) {
			x = access(j->job, 0);
			if(x==-1 && errno==ENOENT)
				tag = "MISSING";
			else if(stat(j->job, &sb)==-1)
				tag = "CANNOT STAT";
			else
				tag = ctime(&sb.st_mtime);
			if(nflag)
				print("%-25s  %19.19s	%s\n", 
					j->job, tag, j->queue);
			if(x==-1 && errno==ENOENT) {
				j->job = 0;
				j->queue = 0;
				qchanged++;
			}
			if(x==0 && qdo(j->job, j->xs)==0) {
				log("jobok", "%s %s\n", j->job, j->xs);
				if(!nflag)
					unlink(j->job);
				j->job = 0;
				j->queue = 0;
				qchanged++;
			}
		}
	if(qchanged && !nflag) {
		fd = creat(pending, 0660);
		for(j=backlog;j; j=j->next) {
			if(j->queue && j->job) {
				log("requeue", "%s %s %s\n", 
					j->queue, j->job, j->xs);
				fprint(fd, "%s	%s %s\n", 
					j->queue, j->job, j->xs);
			}
		}
		close(fd);
		(void)xs(pending,1,1);
	}
}


char *
stash(f)
char *f;
{
	int n, a, b;
	char *g;

	g = temp("dat");
	if(!nflag) {
		a = open(f,0);
		b = creat(g, 0660);
		while((n=read(a,cpbuf,sizeof(cpbuf)))>0)
			write(b,cpbuf,n);
		close(b);
		close(a);
	}
	return g;
}


char *
temp(f)
char *f;
{
	strcpy(cpbuf, spooldir);
	strcat(cpbuf, "/");
	strcat(cpbuf, f);
	strcat(cpbuf, "XXXXXX");
	return strdup(mktemp(cpbuf));
}

deljob(s)
char *s;
{
	char *x, f[50];
	
	sprint(f, "%s/job%6.6d", spooldir, atoi(s));
	if(nflag)
		print("rm %s\n", f);
	else {
		x = xs(f,0,0);
		unlink(f);
		log("deljob", "%s %s %E\n", f, x);
	}
	sprint(f, "%s/dat%6.6d", spooldir, atoi(s));
	if(nflag)
		print("rm %s\n", f);
	else {
		unlink(f);
	}
}

char *
Strdup(s)
char *s;
{
	if(s==0)error("error: missing string", "");
	return strdup(s);
}
