#include <stdio.h>
# define move(x, y) close(y); dup(x); close(x);
char refdir[50];
savedir()
{
	int pipev[2];
	int status, n;

	if (refdir[0]==0) {
		pipe (pipev); 
		if (fork()==0)
		{
			close (pipev[0]); 
			move (pipev[1], 1);
			execl("/bin/pwd", "pwd", 0);
			fprintf(stderr,"Can't run /bin/pwd");
			exit(1);
		}
		close(pipev[1]); 
		wait(&status);
		if (status)
			err("Can't run pwd");
		n = read (pipev[0], refdir, 49);
		refdir[n]=0;
		close(pipev[0]);
		trimnl(refdir);
	}
}
restodir()
{
	chdir(refdir);
}
