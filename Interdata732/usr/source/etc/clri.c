/*
 *	clri inumber filesystem
 */

int zeroes[16];

main(argc, argv)
char *argv[];
{
	register fd, inode;

	if (argc != 3) {
		printf("Arg count\n");
		exit(1);
	}
	if ((fd = open(argv[2], 2)) < 0) {
		printf("Can't open %s\n", argv[2]);
		exit(1);
	}
	if ((inode = atoi(argv[1])-1) < 0) {
		printf("Illegal i-number\n");
		exit(1);
	}
	seek(fd, 1024+inode*64, 0);
	write(fd, zeroes, 64);
}
