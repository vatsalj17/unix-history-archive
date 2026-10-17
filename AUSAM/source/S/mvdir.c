#include	<stat.h>

char buf[256];

main(ac, av)
char **av;
{
	register char *p, *q;
	register flag;

	flag = 1;
	if (ac > 2 && av[1][0] == '-' && av[1][1] == 0)
	{
		av++;
		ac--;
		flag = 0;
	}
	if (ac != 3)
	{
		printf("Usage: mvdir [-] old_dir_name new_dir_name\n");
		return 1;
	}
	if (stat(av[1], buf) == -1) eek(av[1]);
	if ((buf->i_mode & IFMT) != IFDIR)
	{
		printf("%s not a directory\n", av[1]);
		return 1;
	}
	printf(" ln %s %s\n", av[1], av[2]);
	if (flag && link(av[1], av[2])) eek(av[2]);
	printf(" unlink %s\n", av[1]);
	if (flag && unlink(av[1]) == -1) eek(av[1]);
	q = par1(av[2]);
	printf(" unlink %s\n", q);
	if (flag && unlink(q) == -1) eek(q);
	p = par2(av[2]);
	printf(" ln %s %s\n", p, q);
	if (flag && link(p, q)) eek(q);
	return 0;
}

par1(s)		/* finds parent of s by bunging on "/.." */
char *s;
{
	register char *p, *q;

	p = buf;
	q = s;
	while ((*p++ = *q++) && p < &buf[sizeof buf] - 3);
	if (*--p)
	{
		printf("%s too long\n", s);
		exit(1);
	}
	*p++ = '/'; *p++ = '.'; *p++ = '.'; *p = 0;
	return buf;
}

par2(s)		/* finds parent of s - not good enough to bung on "/.." here */
char *s;
{
	register char *p, *q;

	p = s;
	q = 0;
	while (*p) if (*p++ == '/') q = p;
	if (q)
	{
		*--q = 0;
		return s;
	}
	return ".";
}

eek(s)
char *s;
{
	perror(s);
	exit(1);
}

#include	<local-system>
#ifdef	SPRINTF
#include	<sprintf.h>
#endif
