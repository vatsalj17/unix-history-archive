#
/*
 *	ianj sep 77
 *
 *	when invoked (only by super-user) will signal all
 *	processes with a '14' (terminate) this will cause
 *	system to shutdown - nicely. only processes not
 *	killed are 0,1,parent and self.  init will loop
 *	waiting for shells to terminate - no restart at all.
 *	parent (presumably shell) can do as wishes.  the
 *	parent can then kill off any remaining processes.
 *	when all shells are dead init will exit - leaving
 *	only process 0 active.
 */

#include	<param.h>
#include	<proc.h>

/*
#define kill(pid, sig)	printf("kill(%u, %u)\n", pid, sig)
	/*
	 * A useful bit of debugging.
	 */

main()
{
	register i;	register struct proc *p;
	register nzomb;
	int llimit = 50;
	unsigned pid,ppid,maxpid;
	int nproc;

	maxpid = 1;
	pid = getpid();	/* my process id */
	nproc = gprocs(proc);

	for(p = &proc[0]; p < &proc[nproc] ; p++ ) {
		if( p->p_stat == 0 ) continue;
		if( p->p_pid == pid ) ppid = p->p_ppid;
	}

	kill(1, SIGTERMINATE);	/* tell init about shutdown */

	/* scan proc array thrice */
	/* kill -14 all but 0,1,pid,ppid first time thru */
	/* on all but first pass only kill new processes (>maxpid) */

	nzomb = 1;	/* force two loops */
	goto start;
	do {
		nice( 20 );	/* after first time thru let them die faster */
		nzomb = 0;
  start:
		i = maxpid;
		for(p = &proc[2]; p <= &proc[nproc] ; p++ ) {
			if(   p->p_stat == 0
			   || p->p_pid == pid
			   || p->p_pid == ppid
						) continue;
			nzomb++;
			if( p->p_stat == SZOMB ) continue;
			if( maxpid >= p->p_pid ) continue;
			kill( p->p_pid , SIGTERMINATE);
			if( p->p_pid > i ) i = p->p_pid;
		}
		maxpid = i;
		if( nzomb ) nproc = gprocs(proc);
	} while( --llimit && nzomb );
	execl("/bin/ps","ps","gzcpxal",0);
	write(2,"exec ps failed\n",15);
}
