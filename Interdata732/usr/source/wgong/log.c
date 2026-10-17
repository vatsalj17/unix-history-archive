/*
 *		Terminal i/o logger
 */

struct {
	int p_in;
	int p_out;
} fp[2];

int log;
char buff[128];

main(argc, argv)
char **argv;
{
	int stat;

	if (--argc <= 0) {
		log = dup(1);
		close(1);
		dup(2);
	}
	else if ((log = creat(*++argv, 0666)) < 0)
		error("Can't create %s", argv);

	pipe(&fp[0]);
	pipe(&fp[1]);

	signal(2, 1);
	signal(3, 1);

	if (!fork()) {
		close(1);
		dup(fp[0].p_out);
		close(fp[1].p_out);
		pipeline();
	}
	close(fp[0].p_out);
	if (!fork()) {
		close(0);
		dup(fp[0].p_in);
		close(1);
		dup(fp[1].p_out);
		execl("/bin/sh", "-", 0);
		error("Can't exec shell");
	}
	close(fp[1].p_out);
	close(0);
	dup(fp[1].p_in);
	pipeline();
}

pipeline()
{
	register n;

	while ((n = read(0, buff, sizeof buff)) > 0) {
		write(1, buff, n);
		write(log, buff, n);
	}
	exit();
}

error(s, x)
{
	printf(s, x);
	putchar('\n');
	exit(1);
}
