/*
 * fix system image for I/D space
 *  Move data down to 0; move text to 4K.
 *  Also put the data at the start of the
 *  file and the text after it.
 */

#define	words(X)	((X>>1)&077777)
int	tbuf[259];
int	rbuf[259];
int	obuf[259];
long itol();
int	txtsiz;
long	ltxtsiz;
int	datsiz;
long	ldatsiz;
int	bsssiz;
int	symsiz;

char	pwbname[9];
int	pwbflag, pwbseek;
int	txtrel	8192;
int	datrel;
int	symfd;
char	symname[9];

main(argc, argv)
char **argv;
{
	register word, rel, s;

	if (argc<3) {
		printf("Usage: sysfix a.out unix [pwbname]\n");
		exit(1);
	}
	if ((tbuf[0] = open(argv[1], 0)) < 0) {
		printf("Cannot open input %s\n",argv[1]);
		exit(1);
	}
	rbuf[0] = open(argv[1], 0);
	symfd = open(argv[1], 0);
	if ((fcreat(argv[2], obuf)) < 0) {
		printf("Cannot create output %s\n",argv[2]);
		exit(1);
	}
	if (getw(tbuf) != 0407) {
		printf("Bad input format\n");
		exit(1);
	}
	txtsiz = getw(tbuf);
	ltxtsiz = itol(0, txtsiz);
	datsiz = getw(tbuf);
	ldatsiz = itol(0, datsiz);
	bsssiz = getw(tbuf);
	symsiz = getw(tbuf);
	getw(tbuf);
	getw(tbuf);
	if (getw(tbuf) != 0) {
		printf("No relocation bits\n");
		exit(1);
	}
	if ((words(datsiz)+words(bsssiz))>(6*4096)) {
		printf("combined data and bss size too big\n");
		exit(1);
	}
	if (words(txtsiz)>(7*4096)) {
		printf("text size too big\n");
		exit(1);
	}
	if ((words(txtsiz)+words(datsiz)+words(symsiz))>((7*4096)-512)) {
		printf("combined program size too big for boot loader\n");
		exit(1);
	}
	putw(0407, obuf);
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
	lseek(rbuf[0], 020+ltxtsiz+ltxtsiz+ldatsiz, 0);
	s = datsiz >> 1;
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
	seek(rbuf[0], 020+txtsiz+datsiz, 0);
	seek(tbuf[0], 020, 0);
	s = words(txtsiz);
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
	lseek(tbuf[0], 020+ltxtsiz+ltxtsiz+ldatsiz+ldatsiz, 0);
	s = symsiz;
	while ((s =- 12) >= 0) {
		pwbflag  = 0;
		if (putw(getw(tbuf),obuf)=='_p') pwbflag++;
		if (putw(getw(tbuf),obuf)=='wb') pwbflag++;
		if (putw(getw(tbuf),obuf)=='na') pwbflag++;
		if (putw(getw(tbuf),obuf)=='me') pwbflag++;
		rel = getw(tbuf);
		if (rel==043) pwbflag++;
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
		if (pwbflag==5)
			pwbseek = word;
	}
	fflush(obuf);
	if (pwbseek && argc==4) {
		seek(obuf[0],pwbseek,0);
		seek(obuf[0],020,1);
		strcpy(pwbname, argv[3]);
		write(obuf[0], pwbname, 8);
	}
	close(obuf[0]);
	chmod(argv[2],0744);
}

getrel(r)
{
	int	off;
	switch (r&016) {

	case 02:	/* ref to text */
		return(txtrel);
	case 04:		/* ref to data */
	case 06:		/* ref to bss */
		return(datrel);
	case 0:
		return(0);
	case 010:
		off = ((r>>4) & 07777) * 12;
		lseek(symfd, 020+ltxtsiz+ltxtsiz+ldatsiz+ldatsiz+off,0);
		read(symfd, symname, 8);
		printf("Undefined Symbol: %s\n",symname);
		return(0);
	default:
		printf("Bad relocation %o\n", r);
		return(0);
	}
}
