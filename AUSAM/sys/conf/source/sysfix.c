/*
 * fix system image for I/D space
 *  Move data down to 0; move text to 4K.
 *  Also put the data at the start of the
 *  file and the text after it.
 */

int	tbuf[259];
int	rbuf[259];
int	obuf[259];
unsigned	txtsiz;
unsigned	datsiz;
unsigned	bsssiz;
unsigned	symsiz;

unsigned maxtext 57344;		/* 48Kb */
				/* current loader forces 26Kw max size on
				   text+data, so this limit is irrelevant */
long     maxdata 49152;		/* 48Kb */
long     maxtd   53248;		/* the startup routine in m70.s/m45.s requires
				   the combined size of text+data not to
				   exceed 26Kw */
int      txtrel  8192.;
int datrel;


main(argc, argv)
char **argv;
{
	long x,y;
	register word, rel, s;

	if (argc<3) {
		printf("Arg count\n");
		exit(1);
	}
	if ((tbuf[0] = open(argv[1], 0)) < 0) {
		printf("Input file\n");
		exit(1);
	}
	rbuf[0] = open(argv[1], 0);
	if ((fcreat(argv[2], obuf)) < 0) {
		printf("Output file\n");
		exit(1);
	}
	if (getw(tbuf) != 0407) {
		printf("Bad input format\n");
		exit(1);
	}
	putw(0407, obuf);
	txtsiz = getw(tbuf);
	datsiz = getw(tbuf);
	bsssiz = getw(tbuf);
	/*
	 *	check that the generated UNIX will fit
	 */
	x =  txtsiz;
	x =+ datsiz;
	y =  datsiz;
	y =+ bsssiz;
	printf("\ntext=%l (%d) data+bss=%ld (%ld) text+data=%ld (%ld)\n",
		txtsiz,maxtext-txtsiz,y,maxdata-y,x,maxtd-x);
	s = 0;
	if( txtsiz>maxtext ) { printf("<text too big>  "); s++; }
	if( y>maxdata ) { printf("<data+bss too big>  "); s++; }
	if( x>maxtd ) { printf("<text+data too big>"); s++; }
	putchar('\n');
	if( s ) {
		unlink(argv[2]);
		exit(-1);
	}
	x =+ bsssiz;
	printf("%s: %l+%l+%l=%ld  (%o+%o+%o=%lo)\n\n",argv[2],
		txtsiz,datsiz,bsssiz,x,txtsiz,datsiz,bsssiz,x);
	symsiz = getw(tbuf);
	getw(tbuf);
	getw(tbuf);
	if (getw(tbuf) != 0) {
		printf("No relocation bits\n");
		exit(1);
	}
	putw(txtsiz, obuf);
	putw(datsiz, obuf);
	putw(bsssiz, obuf);
	putw(symsiz, obuf);
	putw(0, obuf);
	putw(0, obuf);
	putw(1, obuf);
	datrel = -txtsiz;
/*
 *  Copy out data first
 */
	tbuf[1] = 0;
	seek(tbuf[0], 020+txtsiz, 0);
#ifndef unsw_orig
	useek(rbuf[0], 020+txtsiz, 0);
	useek(rbuf[0], txtsiz, 1);
	useek(rbuf[0], datsiz, 1);
	s = (datsiz >> 1)&077777;
#endif
#ifdef unsw_orig
	seek(rbuf[0], 020+txtsiz, 0);
	seek(rbuf[0], txtsiz, 1);
	seek(rbuf[0], datsiz, 1);
	s = datsiz >> 1;
#endif
	while (s--) {
		word = getw(tbuf);
		rel = getw(rbuf);
		if (rel&01)
			word =- datrel;
		word =+ getrel(rel);
		putw(word, obuf);
	}
/*
 * Now to the text.
 */
	rbuf[1] = 0;
	tbuf[1] = 0;
	seek(rbuf[0], 020+txtsiz, 0);
#ifndef unsw_orig
	useek(rbuf[0], datsiz, 1);
	useek(tbuf[0], 020, 0);
	s = (txtsiz >> 1)&077777;
#endif
#ifdef unsw_orig
	seek(rbuf[0], datsiz, 1);
	seek(tbuf[0], 020, 0);
	s = txtsiz >> 1;
#endif
	while(s--) {
		rel = getw(rbuf);
		word = getw(tbuf);
		if (rel&01)
			word =- txtrel;
		word =+ getrel(rel);
		putw(word, obuf);
	}
/*
 * The symbol table.
 */
	tbuf[1] = 0;
	seek(tbuf[0], 020+txtsiz, 0);
#ifndef unsw_orig
	useek(tbuf[0], txtsiz, 1);
	useek(tbuf[0], datsiz, 1);
	useek(tbuf[0], datsiz, 1);
#endif
#ifdef unsw_orig
	seek(tbuf[0], txtsiz, 1);
	seek(tbuf[0], datsiz, 1);
	seek(tbuf[0], datsiz, 1);
#endif
	s = symsiz;
	while ((s =- 12) >= 0) {
		putw(getw(tbuf), obuf);
		putw(getw(tbuf), obuf);
		putw(getw(tbuf), obuf);
		putw(getw(tbuf), obuf);
		rel = getw(tbuf);
		putw(rel, obuf);
		word = getw(tbuf);
		switch(rel&07) {
			case 2:
				word =+ txtrel;
				break;

			case 3:
			case 4:
				word =+ datrel;
		}
		putw(word, obuf);
	}
	fflush(obuf);
	close(obuf[0]);
	exit(0);
}

getrel(r)
{
	switch (r&016) {

	case 02:	/* ref to text */
		return(txtrel);

	case 04:		/* ref to data */
	case 06:		/* ref to bss */
		return(datrel);

	case 010:
		printf("Reference to undefined symbol\n");
	case 0:
		return(0);

	default:
		printf("Bad relocation %o\n", r);
		return(0);
	}
}
#ifndef unsw_orig
useek(f,offset,mode)
int f, offset, mode;
{       register int fd,t;
	fd = f;
	t = offset;
	if (t >= 0 || mode == 0 || mode == 3)
		return (seek (fd,offset,mode));
	else
	{       seek (fd,040000,mode);
		seek (fd,040000,mode);
		return (seek(fd,t&077777,mode));
	}
}
#endif
