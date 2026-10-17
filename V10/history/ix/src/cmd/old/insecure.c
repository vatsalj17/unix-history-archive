#include <stdio.h>
#define insecure 104
main(argc,argv)
char **argv;
{
	int x[3];
	syscall(insecure,0,x);
	printf("insecure: old %o %o %o\n",x[0],x[1],x[2]);
	fflush(stdout);
	if(argc == 4) {
		x[0] = otoi(argv[1]);
		x[1] = otoi(argv[2]);
		x[2] = otoi(argv[3]);
		syscall(insecure,1,x);
		syscall(insecure,0,x);
		printf("insecure: new %o %o %o\n",x[0],x[1],x[2]);
	}
	exit(0);
}


otoi(s)
char *s;
{
	int n = 0;
	while(*s) n = n*8 + *s++ - '0';
	return n;
}
