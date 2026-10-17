int buf[256];
main(argc,argv) char **argv;
{
	register int d, i;
	register char *p;
	int uid;

	if (argc == 1) {
		printf("USAGE: usage file [uid1 uid2 ...]  : where 1 < uid < 256\n");
		exit(1);
	}
	argc--;
	argv++;
	p = *argv;
	argc--;
	if ((d=open(p,0))<0) {
		printf("file %s not present\n",p);
		exit(1);
	}
	read(d,buf,512);
	printf("statistics for file %s from %s\n\n",p,ctime(&buf[0]));
	if (argc <= 0) 
		for(i = 2;i < 256;i =+ 6) {
			printf("uid %-8d: %3d    %3d    ",i,buf[i],buf[i+1]);
			if (i >= 250) {putchar('\n'); break;}
			printf("%3d    %3d    %3d",buf[i+2],buf[i+3],buf[i+4]);
			printf("    %3d\n",buf[i+5]);
		}
	else
		while (argc--) {
			argv++;
			if (((uid = atoi(*argv)) < 2) || (uid > 255)) {
				printf("out of range: uid's from 2 to 255\n");
				exit(1);
			}
			printf("uid %-8s: %d\n",*argv,buf[uid]);
		}
	close(d);
}
