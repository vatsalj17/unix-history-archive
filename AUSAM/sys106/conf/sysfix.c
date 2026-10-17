/*
 * fix system image for I/D space
 *  Move data down to 0; move text to absolute address 8K
 *	(i.e. forget first instruction page).
 *  Put the data at the start of the file and the text after it.
 *  Remove relocation information.
 */

/*
 *	The following limits are very dependent on your version of
 *	m45.s/m70.s, and your bootstrap loader.
 */
unsigned maxtext	49152;		/* 48Kb */
long	 maxdb		40960;		/* 40Kb */
long	 maxtd		55296;		/* 54Kb */

#define	 TXTREL		8192		/* 8Kb */

int	ibuf[259];
int	rbuf[259];
int	obuf[259];
unsigned	txtsiz;
unsigned	datsiz;
unsigned	bsssiz;
unsigned	symsiz;


main(argc, argv)
char **argv;
{
	long x,y;
	register word, rel, s;

	if (argc < 3)
	{
		printf("Arg count\n");
		exit(1);
	}
	if ((ibuf[0] = open(argv[1], 0)) < 0)
	{
		printf("Input file\n");
		exit(1);
	}
	rbuf[0] = open(argv[1], 0);
	if ((fcreat(argv[2], obuf)) < 0)
	{
		printf("Output file\n");
		exit(1);
	}
	if (getw(ibuf) != 0407)
	{
		printf("Bad input format\n");
		exit(1);
	}
	putw(0407, obuf);
	txtsiz = getw(ibuf);
	datsiz = getw(ibuf);
	bsssiz = getw(ibuf);
	/*
	 *	check that the generated UNIX will fit
	 */
	x =  txtsiz;
	x =+ datsiz;
	y =  datsiz;
	y =+ bsssiz;
	printf("\ntext=%d (max %d) data+bss=%D (max %D) text+data=%D (max %D)\n",
		txtsiz, maxtext, y, maxdb, x, maxtd);
	s = 0;
	if (txtsiz > maxtext)
	{
		printf("<text too big>  ");
		s++;
	}
	if (y > maxdb)
	{
		printf("<data+bss too big>  ");
		s++;
	}
	if (x > maxtd)
	{
		printf("<text+data too big>");
		s++;
	}
	putchar('\n');
	if (s)
	{
		unlink(argv[2]);
		exit(-1);
	}
	symsiz = getw(ibuf);
	getw(ibuf);		/* entry point */
	getw(ibuf);		/* shared data ID */
	if (getw(ibuf) != 0)
	{
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
/*
 *  Copy out data first
 */
	ibuf[1] = 0;			/* nleft - to fool getw */
	seek(ibuf[0], 020+txtsiz, 0);	/* start of data */
	seek(rbuf[0], 020 + txtsiz, 0);
	useek(rbuf[0], datsiz);
	useek(rbuf[0], txtsiz);		/* data relocation bits */
	s = (datsiz >> 1) & 077777;
	while (s--)
	{
		word = getw(ibuf);
		rel = getw(rbuf);
		if (rel & 01)		/* pc relative reference */
			word =+ txtsiz;
		word =+ getrel(rel);
		putw(word, obuf);
	}
/*
 * Now to the text.
 */
	rbuf[1] = 0;			/* nleft - to fool getw */
	ibuf[1] = 0;			/* nleft - to fool getw */
	seek(ibuf[0], 020, 0);		/* start of text */
	seek(rbuf[0], 020 + txtsiz, 0);
	useek(rbuf[0], datsiz);		/* text relocation bits */
	s = (txtsiz >> 1)&077777;
	while (s--)
	{
		rel = getw(rbuf);
		word = getw(ibuf);
		if (rel & 01)		/* pc relative reference */
			word =- TXTREL;
		word =+ getrel(rel);
		putw(word, obuf);
	}
/*
 * The symbol table.
 */
	ibuf[1] = 0;			/* nleft - to fool getw */
	seek(ibuf[0], 020 + txtsiz, 0);
	useek(ibuf[0], datsiz);
	useek(ibuf[0], txtsiz);
	useek(ibuf[0], datsiz);
	s = symsiz;
	while ((s =- 12) >= 0)
	{
		putw(getw(ibuf), obuf);
		putw(getw(ibuf), obuf);
		putw(getw(ibuf), obuf);
		putw(getw(ibuf), obuf);
		rel = getw(ibuf);
		putw(rel, obuf);
		word = getw(ibuf);
		switch(rel & 07)
		{
	    case 2:			/* text symbol */
			word =+ TXTREL;
			break;

	    case 3:			/* data symbol */
	    case 4:			/* bss symbol */
			word =- txtsiz;
		}
		putw(word, obuf);
	}
	fflush(obuf);
	close(obuf[0]);
	exit(0);
}

getrel(r)
{
	switch (r & 016)
	{

    case 02:			/* ref to text */
		return(TXTREL);

    case 04:			/* ref to data */
    case 06:			/* ref to bss */
		return(-txtsiz);

    case 010:
		printf("Reference to undefined symbol\n");
    case 0:			/* absolute reference */
		return(0);

    default:
		printf("Bad relocation 0%o\n", r);
		return(0);
	}
}

/*
 *	This special seek routine is necessary for relative seeks
 *	as the offset is then taken (by the system) to be SIGNED
 */
useek(fd, offset)
register offset;
{
	if (offset < 0)
	{
		seek(fd, 040000, 1);
		seek(fd, 040000, 1);	/* 040000 + 040000 = ... */
	}
	seek(fd, offset & 077777, 1);
}

#include	<local-system>
#ifdef	SPRINTF
#define	NUMBERS
#define	OCTAL
#define	LONG
#include	<sprintf.h>
#endif
