#define	NMOUNT	20

struct mtab {
	char fsys[10];
	char node[10];
	int  ro_flg;
	int  date[2];
} mtab[NMOUNT];

main(argc, argv)
char **argv;
{
	register struct mtab *mp;
	register char *np, *np1;
	int rec;

	sync();
	rec = open("/etc/mnttab",0);
	read(rec, mtab, sizeof mtab);
	if(argc != 2) {
		printf("arg count\n");
		exit(1);
	}
	if (umount(argv[1]) < 0) {
		perror("umount");
		exit(1);
	}
	np = argv[1];
	while(*np++);
	np--;
	while(*--np == '/') *np = '\0';
	while(np > argv[1] && *--np != '/');
	if(*np == '/') np++;
	argv[1] = np;
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		np = argv[1];
		np1 = mp->fsys;
		while(*++np == *++np1)
			if(*np1 == '\0') {
				mp->fsys[0] = '\0';
				time(mp->date);
				mp = &mtab[NMOUNT];
				while ((--mp)->fsys[0] == '\0');
				rec = creat("/etc/mnttab", 0644);
				write(rec, mtab, (mp-mtab+1)*sizeof mtab[0]);
				exit(0);
			}
	}
	printf("/dev/%s not in mount table\n", argv[1]);
	exit(1);
}
