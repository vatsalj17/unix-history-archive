#include <signal.h>

static	ringring;

sleep(n)
unsigned n;
{
	int sleepx();
	unsigned altime;
	int (*alsig)() = SIG_DFL;

	if (n==0)
		return;
	altime = alarm(1000);	/* time to maneuver */
	if (altime) {
		if (altime > n)
			altime -= n;
		else {
			n = altime;
			altime = 1;
		}
	}
	alsig = signal(SIGALRM, sleepx);
	alarm(n);
	for(;;) {
		pause();
		if (ringring) {
			signal(SIGALRM, alsig);
			alarm(altime);
			ringring = 0;
			return;
		}
	}
	/*NOTREACHED*/
}

static
sleepx()
{
	ringring = 1;
}
