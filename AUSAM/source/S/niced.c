#include	<param.h>
#include	<proc.h>

main(c, v)
char *v[];
{
	register i,j,n;	int flag = 0;
	int mypid = getpid();

	if(c == 2) n = atoi(v[1]);
	i = gprocs(proc);

	for(j = 0; j< i; j++)
	{
		if( (proc[j].p_stat == SRUN && proc[j].p_nice == 0 &&
			proc[j].p_pid != mypid) ||
			  ( n != 0 && proc[j].p_pid == n)) {
			printf("proc %d at _proc+%o, pid = %d, uid = %d\n",
				j, j*sizeof proc[0], proc[j].p_pid,
					proc[j].p_uid);
			flag++;
		}
	}
	if( flag ) printf(" nice at %o\n", &(0->p_nice));
}
