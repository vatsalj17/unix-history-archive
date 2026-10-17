#include "util.h"
#include <pwd.h>
#include <sys/stat.h>
#include <utmp.h>

extern int errno;
extern jmp_buf timerest;
extern char *blt();

FILE *tty_fp;                     /* user sends tty output to this      */

/*  ****************  (tty_) ACCESS USERS' TTYS  ********************** */

tty_init ()                       /* setup for ttyecipient's tty        */
{
    setutmp ();                   /* get the utmp file                  */
    return (OK);
}

tty_end ()                       /* done with tty access               */
{
    endutmp ();                   /* done with utmp package             */
    return (OK);
}

tty_find (usrname)                /* find & open recipient's tty        */
char    usrname[];
{
    extern struct utmp *getutnam ();
    static char tty_num[14] = "/dev/";
				  /* number of tty receiver is on       */
				  /* name is filled in from getutnam    */
    struct utmp *utptr;

    if ((utptr = getutnam (usrname)) == 0)
    {
	errno = ENOENT;           /* not logged in                      */
	return (NOTOK);
    }
    blt (utptr -> ut_line, &tty_num[5], 8);

    return (tty_open (tty_num));
}
/**/

LOCFUN
	tty_open (tty_num)        /* open the named tty                 */
char tty_num[];
{
    struct stat statbuf;
    int tty_fd;

    if (setjmp (timerest))   /* signal, for timerest, must be      */
    {                             /*    set by user program             */
	errno = EBUSY;            /* allow a later try                  */
	return (NOTOK);           /* timeout during tty open            */
    }

#ifndef SECURETTY
    if (stat (tty_num, &statbuf) < OK)
	return (NOTOK);

    if (!(statbuf.st_mode & (S_IWRITE >> 6)))
#else
    if (access (tty_num, 01) != 0)
#endif
    {                             /* check write(I) permissions      */
	errno = EBUSY;
	return (NOTOK);
    }

    s_alarm (15);
    tty_fd = open (tty_num, 1);
    s_alarm (0);

    if (tty_fd < 0)
	return (NOTOK);

    tty_fp = fdopen (tty_fd, "w");
    return (OK);
}


tty_close ()			  /* done writing to tty                */
{
    if (ferror (tty_fp) || fclose (tty_fp) == EOF)
	return (NOTOK);
    return (OK);
}
