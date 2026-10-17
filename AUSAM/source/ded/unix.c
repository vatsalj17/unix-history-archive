#include "ded.h"
#include "signal.h"

char unixbuffer[ENOUGH];       /* remember old commands */

do_unix(comm)
char *comm;
 { register savintr, pid, rpid;
    int oldcom, savquit;
    char c,*lp,*fp;

    oldcom = false;
    lp = unixbuffer;

    if (comm != 0) /* zero parameter means @ command */
     { comm = fillin(comm, unixbuffer, "no command");

	/* copy command into buffer */
	do
	 { if ((c = *comm++) == '%')
	     { oldcom = true;
		fp = filename;
		while ((*lp++ = *fp++));
		  lp--;
	     }
	    else *lp++ = c;
	 } while (c != '\0');

	if(oldcom)
	  { redraw(EDITROW,0,">!");
	     redraw(EDITROW,2,unixbuffer);
	     if (mode==EDMODE) virt_c.col = lastcol(EDITROW)+1;
	     else edit_c.col = lastcol(EDITROW)+1;
	     blobit();
	  }
     }

    /* execute something */
    /* no interrupts in parent */
    savquit = signal(SIGQUIT, 1);
    savintr = signal(SIGINTR, 1);

    if ((pid = fork()) == 0) /* I am now the child */
     { ttyreset(); printf("\n");

	/* set default signal handling in child */
	signal(SIGQUIT, 0);
	signal(SIGINTR, 0);

	if( comm )
	    execl("/bin/sh", "-", "-c", unixbuffer, 0);
	else
	    execl("/usr/new/newsh", "-", 0);
	diag("!?can't find or execute shell?!");
	exit();
     }
    else
    if(pid == -1) /* it failed */
     { diag("try again");
	signal(SIGQUIT, savquit);
	signal(SIGINTR, savintr);
	return;
     }
    else
     {
	pwait(pid);
	signal(SIGINTR, savintr); /* as you were */
	signal(SIGQUIT, savquit);

	/* on return, use ttysetmode so as not to flush the input
	 * buffer - we may have read ahead and will have read ahead
	 * when reading from an old dlog file.
	 */
	if (comm != 0) /* zero parameter means @ command */
	 { printf("(ded here) - type any character to redisplay screen");
	    ttysetmode(); ttyin();
	 }
	else ttysetmode();

	saveedit(); savescreen(); setupscreen();
     }
 }

/* procedure that interprets error signals from activated process */
pwait(i)        /* i - process no */
 { register p, e;
    int s;

    if(i != 0)
      for(;;)
	{  p = wait(&s);
	    if(p == -1) break; /* no children */

	    if(s&0200) runstack(s);

	    if(i == p) break;
	}
}

runstack(s)
 { char *flag;
    register char *sp;
    register pid, rpid;
    int status;

    flag = "-sx";
    flag[2] = s&0377;

    if( (pid = fork()) == 0 )
     { /* actually, stack does its own signals... */
	signal(SIGINTR, 0);
	signal(SIGQUIT, 1); /* no more core dumps...*/
	execl("/usr/bin/stack", "stack", flag, 0);
	exit();
     }
    while( (rpid = wait(&status)) != pid && rpid != -1 );
}
