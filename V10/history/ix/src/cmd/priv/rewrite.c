#include "priv.h"
#include <signal.h>
#include <sys/pex.h>
#include <sys/types.h>
#include <sys/stat.h>


extern long lseek();
static progtrusted;

catch()
{
	unpex(fileno(stdin), -1);
	exit(1);
}

rewrite()
{
	int trusted = istrusted(privs);
	int new;
	register FILE *fnew;
	int infile = fileno(stdin);
	int i;
	struct pexclude pexbuf;
	struct stat sb1, sb2;

	if(nflag) {
		display(stdout);
		return;
	}
	signal(SIGINT, catch);
	if(pexinput() == -1)
		error("can't establish trusted path");
	dump(stderr);
	fwrite("? ", 1, 2, stderr);
	fflush(stderr);		/* for good measure */
	if(fgetc(stdin) != 'y'
	|| fgetc(stdin) != '\n'
	|| unpex(infile, 0) == -1) {
		unpex(infile, 0);
		error("request retracted");
	}

	savelab();
	progtrusted = (plab.lb_t&T_SETLIC)!=0;
	if(trusted && !progtrusted)
		error("program untrusted, file trusted");

	if((new=open(privs,1)) == -1
	|| pex(new, 0, &pexbuf) != 0
	|| fstat(new,&sb1) == -1)
		goto privsbad;
	
	for(i=0; i<NSIG; i++)
		signal(i,SIG_IGN);
	
	setlab(new,  privs, "-");
	close(creat(privs, 0));	/* fcreat(new) [4bsd ftrunc] */
	if(lseek(new,0L,2) != 0)
		goto privsbad;
	fnew = fdopen(new,"w");
	if(fnew==0)
		goto privsbad;
	display(fnew);
	fflush(fnew);
	if(ferror(fnew))
		goto privsbad;
	if(trusted && setlab(new, privs, "- n") == -1) {
		unpex(new, 15);
		error("not trusted");
	}
	if(unpex(new, 15) != 0
	|| fclose(fnew) == -1)
		goto privsbad;
	if(trusted && !istrusted(privs)
	|| stat(privs,&sb2) == -1
	|| sb1.st_dev != sb2.st_dev
	|| sb1.st_ino != sb2.st_ino
	|| sb1.st_mode != sb2.st_mode
	|| sb1.st_uid != sb2.st_uid
	|| sb1.st_gid != sb2.st_gid)
		error("hanky-panky during edit of %s",privs);
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
		perror("");
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
		perror("exec /bin/setlab");
		exit(1);
	default:
		wait(&status);
		if(status != 0)
			fprintf(stderr,"(/dev/stdin=%s)\n",name);
		return status;
	}
}
