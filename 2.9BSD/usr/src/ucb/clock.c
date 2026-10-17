/* Copyright (c) 1979 Regents of the University of California */
/*
 * clock - continually print the time of day
 */
#include	<setjmp.h>
jmp_buf	env;

main()
{
	register char *tp;
	int tim[2];
	extern int stop();

	setjmp(env);
	signal(3, &stop);
	time(tim);
	tp = ctime(tim)+11;
	printf("\n\t%8.8s\t", tp);
	while (1) {
		time(tim);
		tp = ctime(tim)+11;
		printf("%8.8s\b\b\b\b\b\b\b\b",tp);
		sleep(1);
	}
}


stop()
{
	extern int start();
	signal(3, &start);
	while (1)
		sleep(3600);
}

start()
{
	longjmp(env, 1);
}
