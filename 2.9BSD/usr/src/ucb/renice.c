#ifndef	lint
static	char	*sccs_id []	= "@(#)renice.c	1.4	11/22/80";
#endif
/*
 *	renice:	change a process's nice value.  Root can change the value
 *		to anything, others can only increase the current nice.
 */

int	atoi ();

main (ac, av)
register	ac;
register	char	**av;
{
	register	i;

	if (ac < 3)	{
		printf ("Usage:  renice newnice pid [pid2 ... pidn]\n");
		exit(1);
		}

	for(i = 2; i < ac; i++)
		if (renice (atoi (av [i]), atoi (av [1])) == -1)
			perror(av[i]);
}
