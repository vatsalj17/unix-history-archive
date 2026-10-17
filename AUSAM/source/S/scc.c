#include	<defines.h>
#include	<param.h>
#include	<proc.h>

int	sleepin, sleepout,
	waitin,  waitout,
	runin,   runout,
	zomb,
	sleeplock,
	spare,
	inswaps, outswaps;
struct lproc
{
	int	pid;
	int	flag;
}
lasttime[NPROC];
main(ac, av)
char **av;
{
	register struct proc *p;
	register struct lproc *l;
	register n = 0;

#ifdef	PROCESS_QUEUES
	printf("|   Sleeping    |\n");
	printf("|Pri < 0|Pri > 0|Runable|         | Swaps |\n");
#else
	printf("| Asleep|Waiting|Runable|         | Swaps |\n");
#endif	PROCESS_QUEUES
	printf("| In|Out| In|Out| In|Out|Dead|Free| In|Out|\n");
	for(;;)
	{
		sleeplock = 0;
		sleepin = sleepout = 0;
		waitin  = waitout  = 0;
		runin   = runout   = 0;
		zomb = spare = 0;
		inswaps = outswaps = 0;
		l = &lasttime[n];
		n = gprocs(proc);
		p = &proc[n];
		while( l < &lasttime[n])
		{
			l->pid = 0;
			l++;
		}
		l = &lasttime[n];
		do
		{
			p--;
			l--;
			switch(p->p_stat)
			{
		    case 0:
				spare++;
				continue;
		    case SSLEEP:
#ifdef	PROCESS_QUEUES
				if(p->p_pri < 0){
#endif
				switch(p->p_flag & (SLOAD|SLOCK))
				{
			    case (SLOAD|SLOCK):
					sleeplock++;
			    case SLOAD:
					sleepin++;
					break;
			    case 0:
					sleepout++;
					break;
				}
				break;
#ifndef	PROCESS_QUEUES
		    case SWAIT:
#else
				}else{
#endif
				if(p->p_flag & SLOAD)
				{
					waitin++;
				}
				else
				{
					waitout++;
				}
				break;
#ifdef	PROCESS_QUEUES
				}
#endif
		    case SRUN:
				if(p->p_flag & SLOAD)
				{
					runin++;
				}
				else
				{
					runout++;
				}
				break;
		    case SZOMB:
				zomb++;
				continue;
			}
			if(p->p_pid == l->pid)
			{
				p->p_flag =& SLOAD;
				if( p->p_flag != l->flag)
				{
					inswaps =+ p->p_flag;
					outswaps =+ l->flag;
				}
			}
			l->pid = p->p_pid;
			l->flag = p->p_flag;
		}
		while(p != proc);
		spare =+ NPROC - n;
		printf("|%3l|%3l|%3l|%3l|%3l|%3l|%4l|%4l|%3l|%3l|\r",
			sleepin,sleepout, waitin,waitout,
			runin,runout, zomb, spare, inswaps, outswaps);
		sleep(1);
	}
}
