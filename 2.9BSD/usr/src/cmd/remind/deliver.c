#include "remind.h"

deliver()       /* Never returns */
{	register int i,j, ntts;
	char ttlist[10][10];
	int fd[2];

	if (m.dirsize)
	{	pipe(fd);
		if (spawn())
		{	/* Parent - write commands to shell */
			write (fd[1],"chdir ",6);
			write (fd[1],&exdir[1], (m.dirsize&0377)-1);
			write (fd[1], msg, m.msgbytes);
			close (fd[1]);
			close (fd[0]);
		}
		else
		{       /* Child - exec a shell */
			close (0);
			dup (fd[0]);
			close (fd[0]);
			close (1);		/* Dummy output files */
			open (dummy, 2);
			close (2);
			dup (1);
			close (fd[1]);
			if(Uid != -1)
			    setuid(Uid);
			execl(shell," remindsh",0);
			exit(1);
		}
	}

	for (i = m.nrcvrs; i--; )
	{       if(( ntts = nametty( 0,ttlist,rcvrlist[i])) && ( !lflag ))
		{	/* logged on the ntts ttys in ttlist */
			for (j = 0; j < ntts; j++)
			{	if (trysend(rcvrlist[i], ttlist[j]))
					continue;
				else  /* Spawn a proc to try every so often */
				{       if (spawn() != 0) continue;
					while(!trysend(rcvrlist[i],ttlist[j]))
						sleep(30);
					exit(0);
				}
			}
		}
		else	/* original did not include this else statement -- pag 2/7/78 */
			sendmail(rcvrlist[i]);  /* If not logged in */
	}
	exit(0);
}
