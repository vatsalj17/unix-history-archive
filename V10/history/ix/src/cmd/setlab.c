#include <sys/label.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/filio.h>
#include <signal.h>
#include <errno.h>

char *logfile = "/dev/log/setlablog";
extern char *getstsrc();
extern char *strncpy();
extern void exit();

extern struct label labelno;
extern struct label labelbot;
struct label plab, pceil;
struct label arg, old, new;
int aflag;
int sflag;
int pflag;
int vflag;

/* to downgrade files, setlab needs capability T_EXTERN
   to change privileges, it needs capability T_SETPRIV
   to set a file away from NO, it needs T_EXTERN and T_NOCHK
*/

sigpipe() {
	signal(SIGPIPE,sigpipe);
}

main(ac, av)
char **av;
{
	int i;
	int errcnt = 0;
	struct label *lp;
	
	nochk(2,0);
	signal(SIGPIPE,sigpipe);

	for( ; ac>1; ac--,av++) {
		if(strcmp(av[1],"-v")==0)
			vflag++;
		else if(strcmp(av[1],"-p")==0)
			pflag++;
		else if(strcmp(av[1],"-a")==0)
			aflag++;
		else if(strcmp(av[1],"-s")==0)
			sflag++;
		else
			break;
	}

	if(aflag+sflag+pflag > 1) {
		fprint(2,"setlab: extraneous options\n");
		exit(1);
	}
	if(ac < 2) {
		fprint(2,"usage: setlab label [files]\n");
		exit(1);
	} 
	getplab(&plab,&pceil);

	lp = atolab(av[1]);
	if(lp == 0) {
		fprint(2,"%s: syntax error in label\n",av[1]);
		exit(1);
	}
	arg = *lp;
	
	if(ac == 2) 
		errcnt += setlab("/dev/stdin");
	else for(i=2; i<ac; i++) 
		errcnt += setlab(av[i]);
	return errcnt;
}

setlab(s)
char *s;
{
	int f;

	errno = 0;
	f = open(s,0);
	if(f == -1)
		f = open(s,1);
	if(f == -1) {
		error(s,f);
		fprint(2,"%s: cannot open\n",s);
		return 1;
	}
	if(getlab(s,f) == -1) 
		return error(s,f);

	if(aflag)
		addlab();
	else if(sflag)
		sublab();
	else if(pflag) {
		new = old;
		new.lb_t = arg.lb_t;
		new.lb_u = arg.lb_u;
	}
	else 
		new = arg;

	if(!old.lb_t && !old.lb_u && new.lb_t) {
		fprint(2,"%s: locking file for vetting\n",s);
		new.lb_t = 0;
		new.lb_u = T_NOCHK;
	}
	if(vflag)
		fprint(2,"%s: new label %s\n",s,labtoa(&new));
	if(fsetflab(f,&new) == 0)
		return logit(s,f,0);

	/* first try failed; see if it's a downgrade */

	if(errno!=ELAB || labLE(&old,&new) || !labLE(&new,&pceil))
		return logit(s,f,errno);
	if((plab.lb_t&T_EXTERN) == 0)
		return error(s,f);
	if(fsetflab(f,&labelno) == -1)
		return logit(s,f,errno);
	if(fsetflab(f,&new) == -1) {
		logit(s,f,errno);
		fprint(2,"%s: left at label NO\n",s);
		return 1;
	}
	return logit(s,f,0);
}

getlab(s,f)
char *s;
{
	int retval = fgetflab(f,&old);
	if(retval == -1)
		fprint(2,"%s: can't get old label\n",s);
	else if(vflag)
		fprint(2,"%s: old label %s\n",s,labtoa(&old));
	return retval;
}

	/* error preserves errno.  the unrelated action of closing
	   f is here to get rid of lots of curly-bracket clutter
	   elsewhere; nothing will be done with f after error */
error(s,f)
char *s;
{
	int saverror = errno;
	perror(s);
	close(f);
	errno = saverror;
	return 1;
}

addlab()
{
	int i;
	new = labelbot;
	for(i=0; i<sizeof new.lb_bits; i++)
		new.lb_bits[i] = old.lb_bits[i] | arg.lb_bits[i];
	new.lb_fix = old.lb_fix | arg.lb_fix;
	new.lb_t = old.lb_t | arg.lb_t;
	new.lb_u = old.lb_u | arg.lb_u;
}

sublab()
{
	int i;
	new = labelbot;
	for(i=0; i<sizeof new.lb_bits; i++)
		new.lb_bits[i] = old.lb_bits[i] & ~arg.lb_bits[i];
	new.lb_fix = old.lb_fix & ~arg.lb_fix;
	new.lb_t = old.lb_t & ~arg.lb_t;
	new.lb_u = old.lb_u & ~arg.lb_u;
}

int logfd = -2;	

	/* closes f like error(), for logging is last thing done */
logit(s,f,err)
char *s;
{	
	char oldlab[200], newlab[200], name[8];
	char *src;
	struct stat statb;
	long t, time();
	char *ctime();
	static notfirst;

	if((old.lb_t|old.lb_u|new.lb_t|new.lb_u) &&
	   (plab.lb_t&T_SETPRIV) ||	/* change privs */
	   !labLE(&old,&new) &&
	   (plab.lb_t&T_EXTERN) ||	/* downgrade */
	   (plab.lb_t&T_NOCHK)) {	/* see the unseeable */
		if(logfd == -2)
			logfd = open(logfile,1);
		if(logfd == -1) {
			error(logfile,logfd);
			fprint(2,"%s: cannot open\n",logfile);
		}
		getlogname(name);
		strncpy(oldlab,labtoa(&old), sizeof(oldlab)-1);
		strncpy(newlab,labtoa(&new), sizeof(newlab)-1);
		time(&t);
		fstat(f,&statb);
		src = getstsrc(3);
		if(src == 0) src = "";
		if(logfd >= 0 && fprint(logfd,
		   "%.8s %d %.24s %.100s(%o/%d) <%s> <%s> <%s> <%s> err=%d\n",
		   name,getuid(),ctime(&t),s,statb.st_dev,
		   statb.st_ino,src,privtoa(plab.lb_t),
		   oldlab,newlab,err) < 0 && !notfirst++) {
			error(logfile,-1);	/* don't close */
			fprint(2,"%s: cannot write\n",logfile);
		}
	}

	if(err) {
		errno = err;
		return error(s,f);
	}
	close(f);
	errno = err;	/* hygiene */
	return err!=0;
}
