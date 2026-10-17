main(ac, av) 
	char **av;
{
	int i;

	if(ac < 2) m("q");
	else
		for(i=1; i<ac; i++)
			m(av[i]);
	exit(0);
}
m(s)
char *s;
{
	char *mkunique(), *t;

	t = mkunique(s, 0644, (int *)0);
	if(t == 0)
		perror(s);
	else
		printf("%s\n", t);
}
