/* name:
	input

function:
	To accept the text of the message and allow ed, msg
	to be called.

algorithm:
	Read until end-of-file, then ask for a valid command

parameters:
	none

returns:
	With a null terminated letter in the string pointed to by
	the global variable letter.

globals:
	letter	address of the buffer.

calls:
	write	system
	printf
	getchar
	system  system

called by:
	main


 */
/*
**	R E V I S I O N  H I S T O R Y
**
**	03/31/83  GWH	Split the SEND program into component parts
**
**	04/05/83  GWH	Changed execl calls to execlp's.
**
**	11/21/83  GWH	Added directedit option.
**
**	05/29/84  GWH	Fixed a but that allowed drafts created in an edit
**			mode to be deleted if an interrupt was made by
**			mistake. Also fixed a bug which resulted in the
**			copy flag (cflag) being erroneously set.
**
**	04/02/85 M. Vasoll  Added the spelling checker command.
*/

#include "./s.h"
#include "./s_externs.h"

extern  char *verdate;
extern  int errno;
extern	int onint (), onint2(), onint3 ();

input ()
{
    int     infile;
    int     stat;
    char    tempbuf[80];
    int     (*old1) (),
	    (*old2) (),
	    (*old3) ();
    register int    i;

    printf ("SEND  (%s)\n", verdate);

    if (to[0] != '\0')
	printf (shrtfmt, toname, to);
    if (cc[0] != '\0')
	printf (shrtfmt, ccname, cc);
    if (subject[0] != '\0')
	printf (shrtfmt, subjname, subject);
    if (bcc[0] != '\0')
	printf (shrtfmt, bccname, bcc);

    if (toflag)
	getaddr (toname, to, NO, host);
    if (ccflag)
	getaddr (ccname, cc, NO, host);
    if (subjflag)
	gethead (subjname, subject, NO);

    fflush (stdout);
    if (!noinputflag) {
        if( dflag > 0 )
        {
	    drclose ();
    
	    old1 =  signal (SIGHUP, SIG_IGN); /* ignore signals intended for edit */
	    old2 =  signal (SIGINT, SIG_IGN);
	    old3 =  signal (SIGQUIT, SIG_IGN);
    
	    if (fork () == 0) {
		signal (SIGHUP, old1);
		signal (SIGINT, orig);
		signal (SIGQUIT, old3);
		if( dflag == 2 )
		    execlp (editor, editor, drffile, (char *)0);
		else
		    execlp( veditor, veditor, drffile, (char *)0);
		printf ("can't execute\n");
		s_exit (-1);
	    }
	    wait (&stat);
	    body = TRUE;	/* Don't know but assume he created a draft */
	    lastsend = 0;
	    signal (SIGHUP, old1);     /* restore signals */
	    signal (SIGINT, old2);
	    signal (SIGQUIT, old3);
	    dropen (DREND);
	} else {
	    if (inclfile[0] == '\0')
		printf ("Type message; end with CTRL-D...\n\n");
	    else
		printf ("Enter comment; end with CTRL-D...\n\n");
	
	more: 
	    dropen (DREND);
	    fflush (stdout);
	    signal (SIGINT, SIG_IGN);
	    signal (SIGQUIT, SIG_IGN);
    
	    while ((i = read (0, bigbuf, BBSIZE - 1)) > 0) {
		if (i == 2 && strncmp (bigbuf, ".\n", 2) == 0)
		    break;		/* Accept . on a line by itself */
		body = TRUE;
		if (write (drffd, bigbuf, i) != i)
		    s_exit (NOTOK);
	    }
	    if (i < 0)
		s_exit (NOTOK);
    
	    signal (SIGINT, onint);	
	}
    }
    setjmp (savej);
    nsent = 0;

    if (inclfile[0] != '\0')
    {                             /* tack file to end of message        */
	putc('\n', stdout);
	body = TRUE;
	strcpy (bigbuf, inclfile);
	goto doincl;
    }

    for (;;)
    {
	char *cp;

	signal (SIGINT, onint3);
	printf ("Command or ?: ");
	fflush (stdout);
	aborted = FALSE;
	if (fgets (bigbuf, BBSIZE, stdin) == NULL)
	    goto byebye;
	if (cp = index(bigbuf, '\n'))
	    *cp = '\0';

	if( strncmp( "set", bigbuf, 3) == 0){
		char *av[NARGS];

		i = sstr2arg(bigbuf, 20, av, " \t");
		if( i == 1 ) {	/* tell him what the current options are */
			printf(" Copyfile  - '%s'\n", copyfile);
			printf(" Signature - '%s'\n", signature);
			printf(" Aliases   - '%s'\n", aliasfilename);
			printf(" Editor    - '%s'\n", editor);
			printf(" Veditor   - '%s'\n", veditor);
			printf(" Checker   - '%s'\n", checker);
			printf(" Subargs   - '%s'\n", subargs);
			if( dflag > 0 )
			printf(" Directedit option on with %s\n",
				(dflag==1?veditor:editor));
			if(qflag == 0 && cflag == 1)
				printf(" File copy option is on\n");
			if(qflag == 1 && cflag == 0)
				printf(" File copy on confirmation only\n");
			continue;
		}
		if( (i = picko( av[1] ) ) < 0) {
			printf("Option '%s' is invalid.\n", av[1]);
			continue;
		}
		select_o( i, &av[1] );
		continue;
	}
	compress (bigbuf, bigbuf);
				  /* so prefix works on multi-words     */
	if (bigbuf[0] == '\0')
	    continue;             /* just hit newline                   */

	signal (SIGINT, onint);

	for (i = 0; !isnull (bigbuf[i]); i++)
	    bigbuf[i] = uptolow (bigbuf[i]);

	if (prefix ("bcc", bigbuf))
	{
	    getaddr (bccname, bcc, YES, locname);
	    continue;
	}
	if (prefix ("delete body", bigbuf))
	{
	    printf ("Are you sure?");
	    if (!confirm ())
		continue;
	    body = FALSE;
	    drempty ();
	    continue;
	}

	if (prefix ("edit", bigbuf) || prefix("vedit", bigbuf))
	{
	    drclose ();

	    old1 =  signal (SIGHUP, SIG_IGN); /* ignore signals intended for edit */
	    old2 =  signal (SIGINT, SIG_IGN);
	    old3 =  signal (SIGQUIT, SIG_IGN);

	    if (fork () == 0)
	    {
		signal (SIGHUP, old1);
		signal (SIGINT, orig);
		signal (SIGQUIT, old3);
		if( bigbuf[0] == 'e' )
			execlp (editor, editor, drffile, (char *)0);
		else
			execlp( veditor, veditor, drffile, (char *)0);
		printf ("can't execute\n");
		s_exit (-1);
	    }
	    wait (&stat);
	    body = TRUE;	/* assume he created a draft file */
	    lastsend = 0;
	    signal (SIGHUP, old1);     /* restore signals */
	    signal (SIGINT, old2);
	    signal (SIGQUIT, old3);
	    dropen (DREND);
	    continue;
	}

	if (prefix ("file include", bigbuf))
	{
	    printf ("File: ");
	    gather (bigbuf, BBSIZE - 1);
	    if (bigbuf[0] == '\0')
		continue;

doincl:
	    infile = open (bigbuf, 0);
	    if (inclfile[0] != '\0')
	    {                     /* and include file                   */
		inclfile[0] = '\0';
	    }
	    if (infile < 0)
	    {
		printf ("can't open: '%s'\n", bigbuf);
		continue;
	    }
	    dropen (DREND);
	    body = TRUE;
	    while ((i = read (infile, bigbuf, BBSIZE - 1)) > 0)
		write (drffd, bigbuf, i);
	    close (infile);
	    printf (" ...included\n");
	    continue;
	}

	if (prefix ("header edit", bigbuf))
	{
	    struct header *hp;

	    printf ("[RETURN (skip) - (delete) + (append)]\n");
	    getaddr (toname, to, YES, locname);
	    getaddr (ccname, cc, YES, locname);
	    gethead (subjname, subject, YES);
	    if (bcc[0] != '\0')
		getaddr (bccname, bcc, YES, locname);
	    for (hp = headers; hp != NULL; hp = hp->hnext)
		gethead (hp->hname, hp->hdata, YES);
	    nsent = lastsend = 0;
	    continue;
	}

	if (prefix ("input more body", bigbuf))
	{
	    printf ("Continue message; end with CTRL-D...\n");
	    goto more;
	}


	if (prefix ("program run", bigbuf))
	{
	    printf ("Program: ");
	    fflush (stdout);
	    if (gets (tempbuf) == NULL)
		continue;
	    drclose ();

	    /* ignore signals intended for subprocess */
	    old1 = signal (SIGHUP, SIG_IGN);
	    old2 = signal (SIGINT, SIG_IGN);
	    old3 = signal (SIGQUIT, SIG_IGN);

	    if (fork () == 0)
	    {
		signal (SIGHUP, old1);
		signal (SIGINT, orig);
		signal (SIGQUIT, old3);
		execlp ("sh", "send-shell", "-c", tempbuf, (char *)0);
		printf ("can't execute\n");
		s_exit (-1);
	    }
	    wait (&stat);
	    signal (SIGHUP, old1);     /* restore signals */
	    signal (SIGINT, old2);
	    signal (SIGQUIT, old3);
	    dropen (DREND);
	    continue;
	}

	if (prefix ("quit", bigbuf) ||
		prefix ("bye", bigbuf))
	{
    byebye: 
	    if ( body && !nsent)
	    {
		printf ("Quit without sending draft");
		if (confirm ())
		    return;
		else
		    continue;
	    }
	    drclose ();
	    return;
	}

	if (prefix ("review message", bigbuf))
	{
	    struct header *hp;

	    printf (hdrfmt, fromname, signature);
	    i = 1;
	    if (to[0] != '\0')
	    {
		i++;
	    	do_review(toname, to);
	    }
	    else
		if (bcc[0] != '\0')
		{
		    i++;
		    do_review(toname, "list: ;");
		 }

	    if (cc[0] != '\0')
	    {
		i++;
		do_review(ccname, cc);
	    }
	    if (subject[0] != '\0')
	    {
		i++;
		do_review(subjname, subject);
	    }
	    if (bcc[0] != '\0')
	    {
		i++;
		do_review(bccname, bcc);
	    }
	    for (hp = headers; hp != NULL; hp = hp->hnext)
		if (isstr(hp->hdata))
		    do_review(hp->hname, hp->hdata);
	    putc ('\n', stdout);
	    fflush (stdout);

	    if (body) {
		dropen (DRBEGIN);
		inrdwr = TRUE;
		msgreview (++i);
		inrdwr = FALSE;
	    }
	    continue;
	}

	if (prefix ("check spelling", bigbuf))
	{
	    drclose ();

	    old1 =  signal (SIGHUP, SIG_IGN); /* ignore signals intended for cheker */
	    old2 =  signal (SIGINT, SIG_IGN);
	    old3 =  signal (SIGQUIT, SIG_IGN);

	    if (fork () == 0)
	    {
		signal (SIGHUP, old1);
		signal (SIGINT, orig);
		signal (SIGQUIT, old3);
		execlp (checker, checker, drffile, (char *)0);
		printf ("can't execute\n");
		s_exit (-1);
	    }
	    wait (&stat);
	    signal (SIGHUP, old1);     /* restore signals */
	    signal (SIGINT, old2);
	    signal (SIGQUIT, old3);
	    dropen (DREND);
	    continue;
	}

	if (prefix ("post message", bigbuf) ||
		prefix ("send message", bigbuf))
	{
	    if (lastsend && lastsend == body)
	    {
		printf ("Without changing anything");
		if (!confirm ())
		    break;
	    }
	    signal (SIGINT, onint2);
	    if( qflag == 1 ){
		printf(" Do you want a file copy of this message ?  ");
		fflush(stdout);
		fgets(tempbuf, sizeof(tempbuf), stdin );
		if( tempbuf[0] == 'y' || tempbuf[0] == 'Y' || tempbuf[0] == '\n')
			cflag = 1;
	    	else
	    		cflag = 0;
	    }
	    post ();
	    if ( qflag == 1 )
		cflag = 0;

/* *** auto-exit, upon successfull send, since draft is saved *** */

	    goto byebye;
	}

	if (prefix ("?", bigbuf) || prefix("help", bigbuf))
	{
	    printf ("bcc\n");
	    printf ("bye\n");
	    printf ("check spelling\n");
	    printf ("delete body\n");
	    printf ("edit body (using editor)\n");
	    printf ("vedit body (using veditor)\n");
	    printf ("file include\n");
	    printf ("header edit\n");
	    printf ("input more body\n");
	    printf ("program run\n");
	    printf ("quit\n");
	    printf ("review message\n");
	    printf ("send message\n");
	    printf ("set [option] [option value]\n");
	    continue;
	}

	printf (" unknown command (type ? for help)\n");
    }				  /* end of loop */
}

do_review (name, data)            /* send out a header field            */
    char name[],                  /* name of field                      */
	*data;                    /* value-part of field                */
{
    register int ind;
    register char *curptr;

    for (curptr = data, ind = 0; ind >= 0; curptr += ind + 1)
    {
	if ((ind = strindex ("\n", curptr)) >= 0)
	    curptr[ind] = '\0';   /* format lines properly              */

	printf (hdrfmt, (curptr == data) ? name : "", curptr);
	if (ind >= 0)
	    curptr[ind] = '\n';   /* put it back                        */
    }
}
