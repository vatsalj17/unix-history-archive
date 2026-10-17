/* nohup */

main(argc, argv)
char **argv;
{
	if(argc < 2) {
		prints("usage: nohup command arg ...\n");
		exit(1);
	}
	argv[argc] = 0;
	if(argv[argc-1] == -1) /* don't ask why */
		argv[argc-1] = -2;
	signal(1, 1);
	signal(2, 1);
	signal(3, 1);
	if(ttyn(1) != 'x') {
		close(2);
		close(1);
		open("/dev/null", 2);
		open("/dev/null", 2);
	} else {
		close(2);
		dup(1);
	}
	pexec(argv[1], &argv[1]);
	close(2);
	open("/dev/tty", 1);
	prints(argv[1]); prints(": not found\n");
	exit(1);
}
prints(s)
char *s;
{
	while(*s)
	write(2, s++, 1);
}
