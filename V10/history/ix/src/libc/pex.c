#include <signal.h>
#include <errno.h>
#include <sys/ttyio.h>
#include <sys/filio.h>
#include <sys/pex.h>

static int tictoc();
static int _pex();
/*
 * t<0: flush, try
 * t=0: try
 * t>0: try & try again
 *  in all cases, back out with NPX if must
 */
pex(fd, t, bufp)
struct pexclude *bufp;
{
	struct pexclude mybuf;
	return _pex(fd, t, bufp?bufp:&mybuf, FIOPX);
}

unpex(fd, t)
{
	struct pexclude mybuf;
	return _pex(fd, t, &mybuf, FIONPX);
}

static
_pex(fd, t, bufp, cmd)
struct pexclude *bufp;
{
	int x;

	if(t<0)
		ioctl(fd, TIOCXFLUSH, 0);
	for(;;) {
		errno = 0;
		if(t>0) {
			long oldt = time((long*)0);
			
			SIG_TYP af;
			af = signal(SIGALRM, tictoc);
			alarm(t);
			x = ioctl(fd, cmd, bufp);
			signal(SIGALRM, af);
			alarm(0);
			t -= time((long*)0) - oldt;
			if(t<0) t = 0;
		} else
			x = ioctl(fd, cmd, bufp);

		if(t==0 || x!=-1 || errno!=EBUSY)
			break;
		if(t<0) {
			ioctl(fd, TIOCXFLUSH, 0);
			nap(1);
		} else if(t > 0) {
			sleep(1);
			t--;
		}
	}
	if(ioctl(fd, FIOQX, bufp) != 0) {
		if(cmd == FIOPX)
			(void) ioctl(fd, FIONPX, bufp);
		return -1;
	} else
		return (bufp->newnear==cmd) ? 0 : -1;
}

static int
tictoc(){
	;
}
