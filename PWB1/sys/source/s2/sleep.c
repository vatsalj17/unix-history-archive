#define ALARM 14
main(argc, argv)
char **argv;
{
	int c, n;
	char *s;
	extern awake();

	n = 0;
	if(argc < 2) {
		prints("Usage: sleep seconds\n");
		exit(1);
	}
	s = argv[1];
	while(c = *s++) {
		if(c<'0' || c>'9') {
			printf("non-numeric argument\n");
			exit(1);
		}
		n = n*10 + c - '0';
	}
        if (n == 0) exit(0);
	signal(ALARM, awake);
	alarm(n);
	pause();
}

awake() {
	exit(0);
}
prints(s)
char *s;
{
	while(*s)
	write(2, s++, 1);
}
