#ifndef	lint
static	char	sccs_id []	= "@(#)submit.c	1.9	5/5/81";
#endif
#include	<ctype.h>
#include	<signal.h>

extern	errno;

main(ac, av)
register	char **av;
{
	register i, pid;

	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);

	switch (fork ())	{

		case -1:
			error (*av);

		case 0:
			--ac, ++av;
			if (!strcmp ("-p", *av) || isnumber(*av) )	{
				if (**av == '-') { av++; ac--; }
				while (ac--)
					if (submit (pid = atoi (*av++)) == -1)
						perror (*av);
					else	renice (pid, 10);
				exit (errno);
				}
			else	if (submit (getpid ()) == -1)
					error ("submit");
				else	nice (10);

			if (!strcmp ("-c", *av))	{
				execl ("/bin/csh", "csh", "-c", *++av, 0);
				error ("csh");
				}
			else	{
				execvp (*av, av);
				error (*av);
				}

		default:
			exit (0);
	}
}

isnumber(s)
register char	*s;
{
	while (*s)
		if (!isdigit(*s++))
			return(0);
	return(1);
}

error (s)
register	char	*s;
{
	perror (s);
	exit (errno);
}
