/*
 * timelim command
 *
 * execute args with cpu time limit of 'n' seconds
 */

main(c, v)
register char	*v[];
register int	c;
{
	register	t;

	if(c < 3 || v[1][0] != '-')
	{
		prints(2, "Usage: timelim -n command [args]\n");
		exit(1);
	}
	v[c] = 0;
	t = atoi( &v[1][1] );
	tlimit(t);
	execc( v[2], &v[2]);
	prints(2, v[2]);
	prints(2, ": not found\n");
	exit(1);
}
