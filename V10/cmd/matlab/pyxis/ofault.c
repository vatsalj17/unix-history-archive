#include <signal.h>
#include <stdio.h>
#include <setjmp.h>

int	num_msgs;			/* Maximum number of error messages */
int	kfault;				/* Overflows detected */
static jmp_buf fpejbuf;

 void
fp_except (signo)
{
	if (kfault < num_msgs) {
		fprintf(stderr, "Floating-point exception!\n");
		fflush(stderr);
		}
	++kfault;
	longjmp(fpejbuf,1);
	}


/* ofault_	Set up to trap arithmetic exceptions reported by SIGFPE.
 *		Calling this function a second (or third or ...) time
 *		will cause the number of error messages to be printed
 *		to be reset.
 */
ofault_(nummsgs)
int	*nummsgs;
{
	extern int num_msgs;
	extern int kfault;

	num_msgs = *nummsgs;
	kfault = 0;
	setjmp(fpejbuf);
	signal (SIGFPE, fp_except);
}

/* kfault_	Return the number of overflows encountered
 */
int kfault_ ()
{
	extern int kfault;

	return (kfault);
}
