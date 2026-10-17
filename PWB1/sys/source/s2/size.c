#include	<stdio.h>

/*
	size -- determine object size

*/

main(argc, argv)
char **argv;
{
	unsigned buf[010];
	long sum;
	int gorp;
	FILE *f;

	if (argc==1) {
		*argv = "a.out";
		argc++;
		--argv;
	}
	gorp = argc;
	while(--argc) {
		++argv;
		if ((f = fopen(*argv, "r"))==NULL) {
			printf("size: %s not found\n", *argv);
			continue;
		}
		fread(buf, sizeof(buf), 1, f);
		if(buf[0]!=0405 && buf[0]!=0411 && buf[0]!=0410 && buf[0]!=0407) {
			printf("size: %s not an object file\n", *argv);
			fclose(f);
			continue;
		}
		if (gorp>2)
			printf("%s: ", *argv);
		printf("%u+%u+%u = ", buf[1],buf[2],buf[3]);
		sum = buf[1];
		sum =+ buf[2];
		sum =+ buf[3];
		printf("%Db = 0%Ob\n", sum, sum);
		fclose(f);
	}
}
