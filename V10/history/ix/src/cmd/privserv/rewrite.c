#include "priv.h"

static progtrusted;
static struct label plab, pceil;

rewrite()
{
	int trusted = istrusted(privs);
	int new;
	struct pexclude pexbuf;
	struct stat sb1, sb2;
	char yes[2];

	if(nflag) {
		display(1);
		return;
	}
	if(pexinput() == -1)
		bomb("can't establish trusted path");
	dump(ttyfd);
	write(ttyfd, "? ", 2);
	if(read(ttyfd, yes,2)!=2
	    || yes[0] != 'y'
	    || yes[1] != '\n'
	    || unpex(ttyfd, 0) == -1) {
		unpex(ttyfd, 0);
		bomb("request retracted");
	}

	if(getplab(&plab, &pceil)!=0)
		bomb("cannot getplab\n");
	progtrusted = (plab.lb_t&T_SETLIC)!=0;
	if(trusted && !progtrusted)
		bomb("program untrusted, file trusted");

	/*
	 * main thread may be checking priv file dates,
	 * another edit may be doing its thing
	 * rule: allow more sleep time in pex call than
	 * any legit action would take.
	 */
	if((new=open(privs,1)) == -1
	    || pex(new, 15, &pexbuf) != 0
	    || fstat(new,&sb1) == -1)
		goto privsbad;

	setlab(new,  privs, "-");
	close(creat(privs, 0));	/* fcreat(new) [4bsd ftrunc] */
	if(lseek(new,0L,2) != 0)
		goto privsbad;
	if(display(new)!=0)
		goto privsbad;
	if(trusted && setlab(new, privs, "- n") == -1) {
		unpex(new, 15);
		bomb("not trusted");
	}
	if(unpex(new, 15) != 0 || close(new) == -1)
		goto privsbad;
	if(trusted && !istrusted(privs)
	    || stat(privs,&sb2) == -1
	    || sb1.st_dev != sb2.st_dev
	    || sb1.st_ino != sb2.st_ino
	    || sb1.st_mode != sb2.st_mode
	    || sb1.st_uid != sb2.st_uid
	    || sb1.st_gid != sb2.st_gid)
		bomb("hanky-panky during edit of %s",privs);
	return;

privsbad:
	perror(privs);
	unpex(new, -1);
	exit(1);
}

setlab(fd, name, privarg)
char *name, *privarg;
{
	int status;
	if(!progtrusted)
		return 0;
	switch(fork()) {
	case -1:
		bomb("");
		return -1;
	case 0:
		plab.lb_t = 0;
		plab.lb_u = T_SETPRIV;
		if(setplab(&plab,&pceil)==-1) {
			perror("setplab");
			exit(1);
		}
		dup2(fd, 0);
		execl("/bin/setlab","setlab", "-p", privarg, (char*)0);
		bomb("exec /bin/setlab");
		exit(1);
	default:
		wait(&status);
		if(status != 0)
			fprint(1,"(/dev/stdin=%s)\n",name);
		return status;
	}
}
