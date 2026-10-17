#include "util.h"
#include "mmdf.h"

/*  INTERRUPT TRAP SETTING AND CATCHING
 *
 *  Aug, 81 Dave Crocker    major addition to timeout, for conditionally
 *                          using timerest only when non-zero.
 */

#include <signal.h>

extern struct ll_struct   *logptr;

/* *******************  SOFTWARE INTERRUPT TRAPS  ********************* */

#ifndef DEBUG
sig10 ()			  /* signal 10 interrupts to here       */
{
    signal (SIGBUS, SIG_DFL);
    ll_err (logptr, LLOGFAT, "Dying on sig10: bus error");
    sigabort ("sig10");
}

sig12 ()			  /* signal 12 interrupts to here       */
{
    signal (SIGSYS, SIG_DFL);
    ll_err (logptr, LLOGFAT, "Dying on sig12: bad sys call arg");
    sigabort ("sig12");
}
#endif DEBUG

sig13 ()			  /* signal 13 interrupts to here       */
{
    signal (SIGPIPE, SIG_DFL);
    ll_err (logptr, LLOGFAT, "Dying on sig13: pipe write w/no reader");
    sigabort ("sig13");
}
/**/

siginit ()			  /* setup interrupt locations          */
{
    extern int timeout();

#ifndef DEBUG
    signal (SIGBUS, sig10);
    signal (SIGSYS, sig12);
#endif DEBUG
    signal (SIGPIPE, sig13);
    signal (SIGALRM, timeout);    /* in case alarm is set & fires       */
}


/* ***********************  TIME-OUT TRAP  **************************** */

jmp_buf timerest;                 /* where to restore to, from timeout  */
				  /* not external, if not used outside  */
				  /* otherwise, set by return routine   */
int     flgtrest;                 /* TRUE, if timerest has been set     */

timeout ()			  /* alarm timeouts/interrupts to here  */
{
    extern int errno;

    signal (SIGALRM, timeout);    /* reenable interrupt for here        */

    if (flgtrest) {
    	errno = EINTR;
    	flgtrest = 0;
	longjmp (timerest, TRUE);
    }

    ll_log (logptr, LLOGFAT, "ALARM signal");
    return;                       /* probably generate an i/o error     */
}

s_alarm(n)
unsigned int n;
{
    flgtrest = ((n == 0) ? 0 : 1);
    alarm(n);
}

/* ARGSUSED */

sigabort (thesig)
    char thesig[];
{
    signal (SIGHUP, SIG_DFL);
    signal (SIGINT, SIG_DFL);
    signal (SIGILL, SIG_DFL);
    signal (SIGBUS, SIG_DFL);
    signal (SIGSYS, SIG_DFL);
    signal (SIGPIPE, SIG_DFL);
    signal (SIGALRM, SIG_DFL);

#ifdef DEBUG
    abort ();
#endif

    exit (NOTOK);
    /* NOTREACHED */
}
