#
/*
 *	rm file ...
 */

#include <local-system>
#include <stat.h>


main(argc, argv)
char *argv[];
{
	register char *arg;
	register int fflg, rflg;
	int retval = 0;

	fflg = 0;
	rflg = 0;
	while(--argc > 0)
	{

		arg = *++argv;
		if(arg[0] == '-')
		{
			if(arg[1] == 'f')
			{
				fflg++;
				continue;
			}
			if(arg[1] == 'r')
			{
				rflg++;
				continue;
			}
		}

		retval =+ rm(arg, fflg, rflg);
	}
	return retval;
}

rm(arg, fflg, rflg)
char arg[];
{
	register int i, b;
	int buf[20];

	if( stat(arg, buf) )
	{
		perror( arg );
		return 1;
	}
	if((buf->i_mode & IFMT) == IFDIR)
	{
		/* a directory */
		if(rflg)
		{
			i = fork();
			if(i == -1)
			{
				perror("try again");
				return 1;
			}
			if(i)
			{
				/* parent */
				while(wait() != i);
				return 0;
			}
			if(chdir(arg))
			{
				perror(arg);
				exit(1);
			}
#ifdef	AGSM
			execl("/etc/glob", "glob", "rm", "-r",
				fflg? "-f": "*", fflg? "*": 0, 0);
			perror("/etc/glob");
#else
			execl("/bin/sh", "sh", "-c",
				fflg ? "rm -f -r *" : "rm -r *", 0);
			perror("/bin/sh");
#endif
			exit(1);
		}
		printf("%s: directory\n", arg);
		return 1;
	}

	if(!fflg)
	{

		if( access(arg,2) )
		{
			printf("%s: mode 0%o: (y-n) ", arg, buf->i_mode & 07777);
			i = b = getchar();
			while(b != '\n' && b != '\0')
				b = getchar();
			if(i != 'y')
				return 1;
		}
	}
	if( unlink(arg) )
	{
		perror( arg );
		return 1;
	}
	return 0;
}
