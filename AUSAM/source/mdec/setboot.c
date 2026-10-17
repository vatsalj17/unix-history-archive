#
/*
 *	useage:  setboot filesystem bootprogname
 */
int	fso,	fsi,	f,	n,	x;
int	buf[256] { 0,0,0,0,0 };
main(argc, argv)
char **argv;
{
	/*
	 *	check arg count, must be 2
	 */
	if(argc != 3) {
		printf("arg count\n\n	useage:	setboot	filesystem bootprogram\n");
		exit();
	}
	/*
	 *	setup filesystem
	 */
	fso = creat(argv[1], 0666);
	if(fso < 0) {
		printf("%s: cannot create\n", argv[1]);
		exit();
	}
	fsi = open(argv[1], 0);
	if(fsi < 0) {
		printf("%s: cannot open\n", argv[1]);
		exit();
	}
	/*
	 * open boot file, check suitability
	 */
	f = open(argv[2], 0);
	if(f < 0) {
		printf("%s: cannot  open \n", argv[2]);
		exit();
	}
	read(f, buf, 020);
	if(buf[0] != 0407) {
		printf("%s: bad format\n", argv[2]);
		exit();
	}
	n = buf[1]+buf[2];
	if(n > 512) {
		printf("%s: too big\n", argv[2]);
		exit();
	}
	/*
	 *	checks out okay, so write it.
	 */
	if (x=read(f, buf, n)<=0) {
				printf("read error %d",x);
				exit();
	}
	seek(fso, 0, 3);
	if(write(fso, buf, 512) != 512) {
		printf("write error: 0\n");
		exit();
	}
	close(f);
}
