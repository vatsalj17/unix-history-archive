#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include <pwd.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>

/*
 *      Program to perform list additions and requests
 *
 *  Feb 84 Steve Kille    - initial coding
 */

FILE *curfp;                             /* for alias file */
char *basehelpfile = "mlist.help";
char *helpfile;

char buf[LINESIZE];

char listname[LINESIZE];
char filename[LINESIZE];
char manager[LINESIZE];
char *username;
int realid;
int curmode;
int gotargs;

#define FREEMODE 0666
#define PUBMODE 0744
#define PRIVMODE 0644
#define SECRMODE 0600

extern char *locmachine;
extern char *supportaddr;
extern char *cmddfldir;

extern char *compress();
extern char *rindex();
extern char *index();
extern char *mktemp();
extern char *strdup();
extern char *getmailid();
extern char *dupfpath();

main (argc, argv)
    int argc;
    char *argv[];
{
    mmdf_init (argv[0]);
    user_init ();
    arg_init (argc, argv);

    if (gotargs)
	exit (OK);

    printf ("Welcome to the mailing list program\n");
    FOREVER
    {
	printf ("\nType 'h' for list of lists, 'c' to create a list,\n");      printf ("'q' to quit, or the name of the list you wish to adjust");
	printf ("\n\n>> ");
	fflush (stdout);
	if (gets (buf) == NULL)
		exit(OK);
	compress (buf, buf);
	if (strlen (buf) == 0)
	    continue;
	if (strlen (buf) == 1)
	switch (buf [0])
	{
	    case '\n':
		continue;
	    case 'q':
		printf ("Mail list program exiting\n");
		exit (OK);
	    case 'c':
		do_create ();
		continue;
	    case 'h':
		do_help ();
		continue;
	}
	do_list (buf);
    }
}
/**/

user_init ()
{
    struct passwd *pwd, *getpwuid();
    char *getmailid();
    int effid;

    umask (07000);                       /* set maks for creation */

    getwho (&realid, &effid);
    if (realid == effid)
	realid = 0;                     /*full priveledges to MMDF      */

    if ((pwd = getpwuid (realid)) == NULL ||
	(username = getmailid (pwd -> pw_name)) == NULL)
    {
	printf ("Problem identifying user\n");
	exit (NOTOK);
    }
    if (realid == 0)
	printf ("You are a mail super-user\n");

}
/**/


arg_init (argc, argv)
    int argc;
    char *argv[];
{
    register int ind;

    helpfile = dupfpath(basehelpfile, cmddfldir);

    gotargs = FALSE;
    for (ind = 1; ind < argc; ind++)
    {
	if (argv[ind][0] != '-')
	{
	    gotargs = TRUE;
	    do_list (argv[ind]);
	}
	else
	    switch (argv[ind][1])
	    {
		case 'f':
		    helpfile = strdup (argv[++ind]);
		    break;

		case 'c':
		    do_create ();
		    printf ("\nMail list program exiting\n");
		    exit (OK);

		case 'h':
		    do_help ();
		    printf ("\nMail list program exiting\n");
		    exit (OK);
		default:
		    printf ("Invalid argument\n");
		    printf ("usage is: mlist [-ch] [-f file] [arg ....]\n");
		    exit (NOTOK);
	    }
   }
}

/**/

do_help ()
{
    FILE *fp;

    signal (SIGINT, SIG_IGN);

    if ((fp = fopen (helpfile, "r")) == NULL)
    {
	printf ("can't open helpfile '%s'\n", helpfile);
	fflush (stdout);
	return;
    }

    while (fgets (buf, LINESIZE, fp) != NULL)
	fputs (buf, stdout);
    fflush (stdout);

    signal (SIGINT, SIG_DFL);
}


do_create ()
{

    ml_1adr (TRUE, FALSE, (char *) 0, "Mailing list creation request",
		supportaddr);
    ml_txt ("Name of list\n");
    printf ("Give name of list: ");
    fflush (stdout);
    if (gets (buf) == NULL)
	exit(OK);
    ml_txt (buf);
    ml_txt ("\n\nFunction:\n");
    printf ("Give a one line description of the list\n: ");
    fflush (stdout);
    if (gets (buf) == NULL)
	exit(OK);
    ml_txt (buf);
    ml_txt ("\n\nDistributed from:\n");
    printf ("Where is the list currently distributed from?\n: ");
    fflush (stdout);
    if (gets (buf) == NULL)
	exit(OK);
    ml_txt (buf);
    ml_txt ("\n\nMaintainers\n");
    printf ("Who will be responible for the list?\n");
    printf ("You can give multiple addresses, but one must be a local ");
    printf ("login\n: ");
    fflush (stdout);
    if (gets (buf) == NULL)
	exit(OK);
    ml_txt (buf);
    printf ("processing message....\n");
    fflush (stdout);
    if (ml_end (OK) != OK)
    {
       printf ("\nproblem processing rquest.  Try again later\n");
       return;
    }
    printf ("Your request has been passed to an administrator\n");
    printf ("You will be notified in a short time when the list has been created\n");
    printf ("You can then use this program to enter names into the list\n\n");
    fflush (stdout);
}

/**/
do_list (list)
char *list;
{
    char tmpbuf [LINESIZE];
    int ismanager;
    struct stat statbuf;
    char *p,
	*q;
    int fd;
    int i;

    strcpy (listname, list);

    ismanager = (realid == 0 ? TRUE : FALSE);
    sprintf (manager, "%s-request", listname);
    if (aliasfetch(TRUE, manager, buf, 0)==OK)
	strcpy (manager, supportaddr);
    else if (!ismanager)
    {
	p = buf;
	while ((q  = index (buf, ',')) != (char *) 0)
	{
	    *q++ = '\0';
	    if (lexequ (p, username))
	    {
		ismanager = TRUE;
		printf ("You are a manager of list '%s'\n", listname);
	    }
	    p = q;
	}
	if (lexequ (p, username))
	{
	    ismanager = TRUE;
	    printf ("You are a manager of list '%s'\n", listname);
	}
    }

    sprintf (tmpbuf, "%s-outbound", listname);
    if (aliasfetch(TRUE, tmpbuf, buf, 0) != OK &&
	aliasfetch(TRUE, listname, buf, 0) != OK)
    {
	printf ("list '%s' does not exist\n", listname);
	return;
    }

    if ((i = strindex (":include:", buf)) < 0)
    {
	printf ("alias '%s' is managed centrally\n", listname);
	printf ("do you wish to mail a request to change the list");
	if (confirm ())
	   u_req ();
	return;
    }

    q = index (&buf[i], '/');
    p = index (q, '@');
    if (p == (char *) 0)
    {
	if ((p = index (q, ',')) != (char *) 0)
	    *p = '\0';
	strcpy (filename, q);
    }
    else
    {
	*p++ = '\0';
	strcpy (filename, q);
	if ((q = index (p, ',')) != (char *) 0)
	    *q = '\0';
	if (ch_h2chan (p, 1) != OK)
	{
	    printf ("Alias '%s' is expanded on machine '%s'\n",
		listname,  p);
	    if (ismanager && realid != 0)
	    {
		printf ("Log in to '%s' and try again\n", p);
		fflush (stdout);
		return;
	    }
	    printf ("do you wish to mail a request to change the list");
	    if (confirm ())
	       u_req ();
	    return;
	}
    }


    if (stat (filename, &statbuf) < 0)
    {                           /* create filewith correct modes */
	if (!ismanager)
	{
	    printf ("File for list '%s' can only be created by manager\n",
			listname);
	    printf ("Do you want to send a message to the list manager");
	    if (confirm ())
	       u_req ();
	    return;
	}
	printf ("Manager creation of list file\n");
	printf ("Do you want any other users to be able to add / remove\n");
	printf ("their own names");

	if (confirm ())
	{
	   printf ("And anyone else's name");
	   if (confirm ())
		curmode = FREEMODE;
	   else
		curmode =  PUBMODE;
	}
	else
	{
	    printf ("Do you want other users to be able to see who is on the list");
	    if (confirm ())
		curmode = PRIVMODE;
	    else
		curmode = SECRMODE;
	}
	printf ("Creating file '%s'\n",  filename);
	if ((fd = creat (filename, curmode)) < 0)
	{
	    printf ("Unable to create filename '%s'\n", filename);
	    return;
	}
	close (fd);
    }
    else
    {
	curmode = (int) statbuf.st_mode;
	curmode = curmode & 0777;

	if (!(ismanager || curmode == PUBMODE || curmode == FREEMODE))
	{
	    printf ("list '%s' can only be updated by its manager\n",
			listname);
	   if (curmode != SECRMODE)
	   {
		printf ("do you want to see who is on the list");
		if (confirm ())
		{
		    if ((curfp = fopen (filename, "r")) == NULL)
		    {
			printf ("Can't open file '%s'\n", filename);
			return;
		    }
		    while (fgets (buf, LINESIZE, curfp) != NULL)
			fputs (buf, stdout);
		    fclose (curfp);
		}
	   }

	   printf ("do you wish to mail a request to be added");
	   if (confirm ())
	      u_req ();
	    return;
	}

	if ((curfp = fopen (filename, "rw")) == NULL)
	{
	   printf ("Can't open file '%s'\n", filename);
	   return;
	}
    }

    printf ("Adjusting list '%s'\n", listname);
    if (ismanager || curmode == FREEMODE)
	master_adj ();
    else
	u_adj ();

}

/**/
u_req ()
{
    ml_1adr (TRUE, FALSE, (char *) 0, "List change request", manager);
    ml_txt ("Automatic request for addition to list: ");
    ml_txt (listname);
    printf ("You may ask for confirmation of any changes requested.\n");
    printf ("Answering no to the next question will save the list ");
    printf ("administrator's time.\n");
    printf ("Do you require confirmation");
    if (confirm ())
	ml_txt ("\n\nConfirmation requested");
    else
	ml_txt ("\n\nConfirmation NOT requested");
    ml_txt ("\n\nNames:\n\n");
    printf ("sending request to list manager (%s)\n", manager);
    printf ("specify names to be added or removed from list '%s'\n",
		listname);
    printf ("end with .<CR> on a newline\n");
    while (fgets (buf, LINESIZE, stdin) != NULL)
	if (strlen (buf) == 2 && buf[0] == '.')
	    break;
	else
	    ml_txt (buf);
   printf ("Processing message....\n");
   fflush (stdout);
   if (ml_end (OK) != OK)
	printf ("problem sending request\n");
   else
   {
	printf ("Your request has been sent\n");
	printf ("It will be processed in the next few days\n");
   }
}

u_adj ()
{
    printf ("would you like a listing of this list");
    if (confirm ())
    {
	if ((curfp = fopen (filename, "r")) == NULL)
	{
	    printf ("Can't open file '%s'\n", filename);
	    return;
	}
	while (fgets (buf, LINESIZE, curfp) != NULL)
		fputs (buf, stdout);
	fclose (curfp);
    }

    printf ("Do you want to add or remove your name");
    if (!confirm ())
    {
	    printf ("Do you want to send a request to the list maintainer\n");
	    if (confirm ())
		u_req ();
	    else
		printf ("sorry, no other options for mortals\n");
	    return;
    }

    if (u_inlist (username))
    {
	printf ("You (%s) are already in this list\n",
		username);
	printf ("do you wish to be removed? ");
	if (confirm ())
	    u_remove (username);
    }
    else
    {
	printf ("You (%s) are not in this list\n",
		username);
	printf ("do you wish to be added? ");
	if (confirm ())
		u_add (username);
    }
}

/**/

master_adj ()
{
char tmpbuf [LINESIZE];

    v_init ();
    printf ("You are in list manager mode\n");
    FOREVER
    {
	printf ("print list (p), verify list (v), add user (a), remove ");
	printf ("user (r), quit (q)?\n");
	printf ("default is assumed to be user name to be added\n\n>>> ");
	fflush (stdout);
	if (gets (tmpbuf) == NULL){
		v_end ();
		exit(OK);
	}
	compress (tmpbuf, tmpbuf);
	tmpbuf [0] = uptolow (tmpbuf[0]);
	switch (tmpbuf[0])
	{
	    case '\0':
	    case '\n':
		continue;
	    case 'q':
		v_end ();
		return;
	    case 'p':
		if ((curfp = fopen (filename, "r")) == NULL)
		{
		    printf ("Can't open file '%s'\n", filename);
		    return;
		}
		while (fgets (tmpbuf, LINESIZE, curfp) != NULL)
			fputs (tmpbuf, stdout);
		fclose (curfp);
		break;
	    case 'v':
		v_list ();
		break;
	    case 'a':
		printf ("give username to be added: ");
		if (gets (tmpbuf) == NULL){
			v_end ();
			exit(OK);
		}
		compress (tmpbuf, tmpbuf);
		if (u_inlist (tmpbuf))
		    printf ("User '%s' already in list\n", tmpbuf);
		else
		    if (verify (tmpbuf))
			u_add (tmpbuf);
		    else
			printf ("Illegal address '%s'\n", tmpbuf);
		break;

	    case 'r':
		printf ("give username to be removed: ");
		if (gets (tmpbuf) == NULL){
			v_end ();
			exit(OK);
		}
		compress (tmpbuf, tmpbuf);
		if (!u_inlist (tmpbuf))
		    printf ("User '%s' not in list\n", tmpbuf);
		else
		    u_remove (tmpbuf);
		break;

	    default:
		if (u_inlist (tmpbuf))
		    printf ("User '%s' already in list\n", tmpbuf);
		else
		    if (verify (tmpbuf))
		    {
			u_add (tmpbuf);
		    }
		    else
			printf ("Illegal address '%s'\n", tmpbuf);
		break;

	}
    }
}

/**/
u_inlist (user)
char *user;
{

    if ((curfp = fopen (filename, "r")) == NULL)
    {
	printf ("Can't open file '%s'\n", filename);
	return (FALSE);
    }
    while (fgets (buf, LINESIZE, curfp) != NULL)
    {
	buf [strlen(buf) - 1] = '\0';
	if (lexequ (user, buf))
	{
	    fclose (curfp);
	    return (TRUE);
	}
    }
    fclose (curfp);
    return (FALSE);
}


u_add (user)
char *user;
{

    if ((curfp = fopen (filename, "a")) == NULL)
    {
       printf ("Can't open file '%s'\n", filename);
       return;
    }
    printf ("adding user '%s'\n", user);
    fputs (user, curfp);
    fputc ('\n', curfp);
    fclose (curfp);
}

u_remove (user)
char *user;
{
    char *template = "al.XXXXXX";
    char fpath [LINESIZE];
    char *cp;
    FILE  *fp;
    int  fd;

			    /* first make temp file in same dir */
    strcpy (fpath, filename);
    cp = rindex (fpath, '/');
    if (cp++ == 0)
	fpath [0] = '\0';
    else
	*cp = '\0';
    strcat (fpath, template);
    mktemp (fpath);
    if ((fd = creat (fpath, curmode)) < 0)
    {
	printf ("Can't create temp file '%s'\n", fpath);
	return;
    }
    if ((fp = fdopen (fd, "w")) == NULL)
    {
	printf ("Can't open temp file '%s'\n", fpath);
	return;
    }


    if ((curfp = fopen (filename, "r")) == NULL)
    {
	printf ("Can't open file '%s'\n", filename);
	return;
    }
    while (fgets (buf, LINESIZE, curfp) != NULL)
    {
	buf [strlen(buf) - 1] = '\0';
	if (lexequ (buf, user) && strlen (buf) == strlen (user))
	    printf ("removing user '%s'\n", buf);
	else
	{
	    fputs (buf, fp);
	    fputc ('\n', fp);
	}
    }
    fclose (curfp);
    fclose (fp);
    unlink (filename);
    link (fpath, filename);
    unlink (fpath);
}


v_list ()                       /* go throughlist and verify    */
{
    int  donesofar;
    int i;
    char tmpbuf [LINESIZE];


    if ((curfp = fopen (filename, "r")) == NULL)
    {
       printf ("Can't open file '%s'\n", filename);
       return;
    }
    donesofar = 0;
    while (fgets (tmpbuf, LINESIZE, curfp) != NULL)
    {
	tmpbuf [strlen(tmpbuf) - 1] = '\0';
	if (!verify (tmpbuf))
	{
	    printf ("Remove user '%s'", tmpbuf);
	    if (confirm ())
	    {
		fclose (curfp);
		u_remove (tmpbuf);
		if ((curfp = fopen (filename, "r")) == NULL)
		{
		    printf ("Can't open file '%s'\n", filename);
		    return;
		}
		for (i = 0; i < donesofar; i++)
		    if (fgets (tmpbuf, LINESIZE, curfp) == NULL)
			continue;
		continue;
	    }
	}
	donesofar++;
    }
}


/**/
				/* address check stuff          */


v_init ()
{
	struct rp_bufstruct thereply;
	int     len;

	if (rp_isbad(mm_init()) || rp_isbad(mm_sbinit()) ||
	    rp_isbad(mm_winit ((char *)0, "vm", "mmdf")) ||
	    rp_isbad(mm_rrply( &thereply, &len ))) {
		printf("Cannot initialize address checking\n");
		mm_end (NOTOK);
		exit( 8 );
	} else if (rp_isbad(thereply.rp_val)) {
		printf ("verify: %s\n", thereply.rp_line);
		mm_end (NOTOK);
		exit(9);
	}

}

v_end ()
{
	mm_end (NOTOK);
}

verify (addr)
char    *addr;
{
	struct rp_bufstruct thereply;
	int     len;

	printf("%s: ", addr);
	fflush(stdout);

	if (rp_isbad (mm_wadr ((char *)0, addr))) {
		if( rp_isbad( mm_rrply( &thereply, &len ))) {
			printf ("Mail system problem\n");
			mm_end (NOTOK);
			exit( 8 );
		} else {
			printf ("%s\n", thereply.rp_line);
			return (FALSE);
		}

	} else {
		if( rp_isbad( mm_rrply( &thereply, &len ))) {
			printf ("Mail system problem\n");
			mm_end (NOTOK);
			exit (7);
		} else if( rp_isbad( thereply.rp_val )) {
			printf ("%s\n", thereply.rp_line);
			return (FALSE);
		} else {
			printf ("OK\n");
			return (TRUE);
		}
	}
	/*NOTREACHED*/
}

/**/
				/*  various utilities                   */

confirm ()
{
    register char   c;


    printf (" [Confirm] ");
    fflush (stdout);

    c = ttychar ();

    switch (c)
    {
	case 'Y':
	case 'y':
	case '\n':
	    printf ("yes\r\n");
	    fflush (stdout);
	    return (TRUE);

	case 'N':
	case 'n':
	    printf ("no\r\n");
	    return (FALSE);
	default:
	    printf ("Type yes or no.  <CR> defaults to yes\n");
	    return (confirm ());
    }
}


ttychar ()
{
    register int    c;

    fflush (stdout);
    fgets (buf, LINESIZE,  stdin);
    c = buf[0];

    c = toascii (c);    /* get rid of high bit */

    if (c == '\r')
	c = '\n';

    return (c);
}
