main(argc, argv)
char	**argv;
{

	int	dv, fno, sbuf[18], dbuf[11];

	if ((dv = open("/dev", 0)) < 0) {
		write(2, "Cannot open /dev\n", 17);
		exit(1);
	}
	while(--argc) {
		seek(dv, 0, 0);
		if (stat(*++argv, sbuf) == -1) continue;
		fno = sbuf[0];
		while(read(dv, &dbuf[2], 16) == 16) {
			if (dbuf[2] == 0) continue;
			dbuf[0] = '//';
			dbuf[1] = 'de';
			dbuf[2] = 'v/';
			if (stat(&dbuf[0], sbuf) == -1) {
				write(2, "/dev stat error\n", 16);
				exit(1);
			}
			if ((fno != sbuf[6]) || ((sbuf[2] & 060000) !=
				060000)) continue;
			printf("%s %s\n", &dbuf[3], *argv);
		}
	}
}
