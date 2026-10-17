/*
 *		time command
 */

char command[128];
int  args[64];

struct {
	int parenttime[2];
	int usertime[2];
	int systime[2];
} tbuf;

main(argc, argv)
char **argv;
{
	int tbefore[2], tafter[2];
	register i, pid, c;
	register char *p;
	extern fout;

	fout = 2;

	if (argc <= 1)
		exit(1);
	for (i=0; i<9; i++)
		command[i] = "/usr/bin/"[i];
	for (p = *++argv; c = *p; p++)
		if (i < 127)
			command[i++] = c;
	for (i=0; --argc; i++)
		if (i < 63)
			args[i] = *argv++;

	time(tbefore);
	if ((pid = fork()) == 0) {
		execv(command+9, args);
		execv(command+4, args);
		execv(command, args);
		printf("%s not found\n", command+9);
		exit(1);
	}
	signal(2, 1);
	while(wait(0) != pid);
	time(tafter);
	signal(2,0);

	times(&tbuf);
	i = tafter[1] - tbefore[1];
	if (i < 0)
		i = ~i + 1;
	putchar('\n');
	ptime("Real", i * 100);
	ptime("Cpu ", tbuf.usertime[1]);
	ptime("Sys ", tbuf.systime[1]);
}

ptime(label, clicks)
char *label;
{
	register t;

	t = clicks;
	printf("%s", label);
	pdec(' ', t/(3600*100));
	t =% 3600*100;
	pdec(':', t/(60*100));
	t =% 60*100;
	pdec(':', t/100);
	t =% 100;
	pdec('.', t);
	putchar('\n');
}

pdec(ch, n)
{
	putchar(ch);
	putchar(n/10 + '0');
	putchar(n%10 + '0');
}
