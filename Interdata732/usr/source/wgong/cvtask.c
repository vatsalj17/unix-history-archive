/*
 * cvtask: utility to convert osmt task to unix a.out file
 *
 */


int ibuf[64];
int obuf[132];

main(argc, argv)
char *argv[];
{
	register in, progsize, i;

	if ((in=open(argv[1], 0)) < 0)
		exit(1);
	if ((obuf[0]=creat(argv[2], -512)) < 0)
		exit(2);

/* read loader info block to get program size */

	if (read(in, ibuf, 256) != 256)
		exit(3);
	progsize = ibuf[26];

/* write size information on a.out file */

	putw(0407, obuf);
	putw(0, obuf);
	putw(progsize, obuf);
	for (i=1; i<=5; i++)
		putw(0, obuf);

/* copy program to a.out file */

	while (progsize >= 256) {
		read(in, ibuf, 256);
		for (i=0; i<64; i++)
			putw(ibuf[i], obuf);
		progsize =- 256;
	}
	if (progsize > 0) {
		read(in, ibuf, progsize);
		for (i=0; i<progsize/4; i++)
			putw(ibuf[i], obuf);
	}

	fflush(obuf);
	exit(0);
}
