/*
 *	entab [infile [outfile] ]
 *		This program replaces all suitable strings of blanks by tabs
 *		in infile placing result in outfile.
 *		May be used as a filter.
 *		also see 'detab'
 */
#define	EOF	-1
#define	EOLN	'\n'
#define	CR	'\r'
#define	FF	'\f'
#define	TAB	'\t'
#define	BS	'\b'

char ibuf[518], obuf[518];

main(argc, argv)
char *argv[];
{
	register c, icol, ocol;

	if(argc > 3)
	{
		prints(2, "Usage: entab [ infile [ outfile ] ]\n");
		return 1;
	}
	if(argc > 1)
	{
		if(fopen(argv[1], ibuf) == -1)
		{
			perror( argv[1] );
			return 1;
		}
	}
	else
	{
		ibuf[0] = 0;	/* use standard input */
	}
	if(argc > 2)
	{
		if(fcreat(argv[2], obuf) == -1)
		{
			perror( argv[2] );
			return 1;
		}
	}
	else
	{
		obuf[0] = 1;	/* use standard output */
	}
	/* here beginneth the program */ 

	icol = ocol = 0;
	while((c = getc(ibuf)) != EOF)
	{
		switch(c)
		{
		case EOLN:
		case CR:
		case FF:
			put( c);
			icol = ocol = 0;
			break;
		case ' ':
			icol++;
			break;
		case TAB:
			icol = (icol+8)&~07;
			break;
		default:
			while(((ocol+8)&~07) <= icol && ocol+1 != icol)
			{
				put( TAB);
				ocol = (ocol+8)&~07;
			}
			while(ocol < icol)
			{
				put( ' ');
				ocol++;
			}
			put( c);
			if(c == BS)
			{
				icol--;
				ocol--;
			}
			else
			{
				icol++;
				ocol++;
			}
		}
	}
	fflush(obuf);
	return 0;
}

put( c )
char c;
{
	extern errno;

	putc( c , obuf );
	if( errno )
	{
		perror("write");
		exit(1);
	}
}
