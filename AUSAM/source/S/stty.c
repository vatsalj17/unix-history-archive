#
/*
 * set teletype modes
 */

#include	<local-system>

#define	SILENT
	/*
	**	implement flag "-v" for silent changes
	*/


/*
 * terminal types
 */
#ifdef	BASSER
#define	ASR33
#define	NCR260
#define	DIABLO
#define	CDC713
#else
#define	TI700
#define	TEKTRONICS
#define	VT05
#endif	BASSER

/*
 * tty flags
 */
#define	HUPCL	01
#define	XTABS	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define	ANYP	0300
#define	INVCASE	04000
#define	FLOWCNTRL 0100000

/*
 * Delay algorithms
 */
#define	CR0	0
#define	CR1	010000
#define	CR2	020000
#define	CR3	030000
#define	NL0	0
#define	NL1	000400
#define	NL2	001000
#define	NL3	001400
#define	TAB0	0
#define	TAB1	002000
#ifndef	INVCASE
#define	TAB2	004000
#define	TAB3	006000
#endif
#ifdef	INVCASE
#define	TAB3	TAB1
#endif
#define	FF0	0
#define	FF1	040000
#ifndef	FLOWCNTRL
#define	BS0	0
#define	BS1	0100000
#endif
#ifndef	INVCASE|FLOWCNTRL
#define	ALL	0177400
#endif
#ifdef	INVCASE|FLOWCNTRL
#define	ALL	073400
#endif

/*#define NO_REDIRECT /* define to stop non-susers from effecting other ttys */
	/*
	 *	disable redirection except for super-user.
	 *	use fd 2 not 1
	 *
	 */
struct
{
	char	*string;
	int	speed;
} speeds[]
{
	"0",	(0<<8)|0,
	"50",	(1<<8)|1,
	"75",	(2<<8)|2,
	"110",	(3<<8)|3,
	"134.5",(4<<8)|4,
	"150",	(5<<8)|5,
	"200",	(6<<8)|6,
	"300",	(7<<8)|7,
	"600",	(8<<8)|8,
	"1200",	(9<<8)|9,
	"1800",	(10<<8)|10,
	"2400",	(11<<8)|11,
	"4800",	(12<<8)|12,
	"9600",	(13<<8)|13,
	"exta",	(14<<8)|14,
	"extb",	(15<<8)|15,
	"134",	(4<<8)|4,
	0,
};
struct
{
	char	*string;
	int	set;
	int	reset;
} modes[]
{
#ifdef	FLOWCNTRL
	"fc",
	FLOWCNTRL, 0,

	"-fc",
	0, FLOWCNTRL,
#endif

#ifdef	INVCASE
	"INVCASE",
	INVCASE, 0,

	"invcase",
	INVCASE, 0,

	"-INVCASE",
	0, INVCASE,

	"-invcase",
	0, INVCASE,
#endif

	"even",
	EVENP, 0,

	"-even",
	0, EVENP,

	"odd",
	ODDP, 0,

	"-odd",
	0, ODDP,

	"raw",
	RAW, 0,

	"-raw",
	0, RAW,

	"cooked",
	0, RAW,

	"-nl",
	CRMOD, 0,

	"nl",
	0, CRMOD,

	"echo",
	ECHO, 0,

	"-echo",
	0, ECHO,

	"LCASE",
	LCASE, 0,

	"lcase",
	LCASE, 0,

	"-LCASE",
	0, LCASE,

	"-lcase",
	0, LCASE,

	"-tabs",
	XTABS, 0,

	"tabs",
	0, XTABS,

	"hup",
	HUPCL, 0,

	"-hup",
	0, HUPCL,

	"cr0",
	CR0, CR3,

	"cr1",
	CR1, CR3,

	"cr2",
	CR2, CR3,

	"cr3",
	CR3, CR3,

	"tab0",
	TAB0, TAB3,

	"tab1",
	TAB1, TAB3,

#ifndef	INVCASE
	"tab2",
	TAB2, TAB3,

	"tab3",
	TAB3, TAB3,
#endif

	"nl0",
	NL0, NL3,

	"nl1",
	NL1, NL3,

	"nl2",
	NL2, NL3,

	"nl3",
	NL3, NL3,

	"ff0",
	FF0, FF1,

	"ff1",
	FF1, FF1,

#ifndef	FLOWCNTRL
	"bs0",
	BS0, BS1,

	"bs1",
	BS1, BS1,
#endif

#ifdef	ASR33
	"33",
	CR1, ALL,

	"tty33",
	CR1, ALL,
#endif

#ifdef	ASR37
	"37",
	FF1+CR2+TAB1+NL1, ALL,

	"tty37",
	FF1+CR2+TAB1+NL1, ALL,
#endif

#ifdef	VT05
	"05",
	NL2, ALL,

	"vt05",
	NL2, ALL,
#endif

#ifdef	TERMINET
	"tn",
	CR1, ALL,

	"tn300",
	CR1, ALL,
#endif

#ifdef	TI700
	"ti",
	CR2, ALL,

	"ti700",
	CR2, ALL,
#endif

#ifdef	TEKTRONICS
	"tek",
	FF1, ALL,
#endif

#ifdef	NCR260
	"NCR",
	NL2+CR3+INVCASE, ALL+LCASE,
	"ncr",
	NL2+CR3+INVCASE, ALL+LCASE,
	"260",
	NL2+CR3+INVCASE, ALL+LCASE,
#endif	NCR260

	0,
};

char	*arg;
int	mode[3];
#ifdef	SILENT
int	vflg;
#endif	SILENT

struct { char lobyte, hibyte; };

main(argc, argv)
char	*argv[];
{
	int i;

	gtty(1, mode);
	if(argc == 1) {
		prmodes();
		exit(0);
	}
	while(--argc > 0) {

		arg = *++argv;
		if (eq("ek"))
#ifndef	BASSER
			mode[1] = '#@';
#else
			mode[1] = '\b@';
#endif	BASSER
		if (eq("erase")) {
			mode[1].lobyte = **++argv;
			argc--;
		}
		if (eq("kill")) {
			mode[1].hibyte = **++argv;
			argc--;
		}
#		ifdef	SILENT
		if (eq("-v"))
			vflg++;
#		endif	SILENT
		for(i=0; speeds[i].string; i++)
			if(eq(speeds[i].string))
				mode[0] = speeds[i].speed;
		for(i=0; modes[i].string; i++)
			if(eq(modes[i].string)) {
				mode[2] =& ~modes[i].reset;
				mode[2] =| modes[i].set;
			}
		if(arg)
			printf("unknown mode: %s\n", arg);
	}
#ifdef NO_REDIRECT
	if(!getreal())
		stty(1,mode);
	else
		stty(2,mode);
#else
	stty(1,mode);
#endif NO_REDIRECT
	gtty(1,mode);
#	ifdef	SILENT
	if ( !vflg )
#	endif	SILENT
	prmodes();
}

eq(string)
register char	*string;
{
	register	i;

	if(!arg)
		return(0);
	i = 0;
loop:
	if(arg[i] != string[i])
		return(0);
	if(arg[i++] != '\0')
		goto loop;
	arg = 0;
	return(1);
}

prmodes()
{
	register m;

	if(mode[0].lobyte != mode[0].hibyte) {
		prspeed("input speed ", mode[0].lobyte);
		prspeed("output speed", mode[0].hibyte);
	} else
		prspeed("speed", mode[0].lobyte);
	printf("erase = '%c'; kill = '%c'\n", mode[1].lobyte, mode[1].hibyte);
	m = mode[2];
#ifdef	FLOWCNTRL
	if ( m&FLOWCNTRL )  printf( "fc " );
#endif
#ifdef	INVCASE
	if ( m&INVCASE )  printf( "invcase " );
#endif
	if(m & 0200) printf("even ");
	if(m & 0100) printf("odd ");
	if(m & 040) printf("raw ");
	if(m & 020) printf("-nl ");
	if(m & 010) printf("echo ");
	if(m & 04) printf("lcase ");
	if(m & 02) printf("-tabs ");
	if(m & 01) printf("hup ");
	delay(m>>8, "nl");
#ifndef	INVCASE
	delay(m>>10, "tab");
#endif
#ifdef	INVCASE
	delay( (m>>10)&1 , "tab" );
#endif
	delay(m>>12, "cr");
	delay((m>>14)&1, "ff");
#ifndef	FLOWCNTRL
	delay((m>>15)&1, "bs");
#endif
	printf("\n");
}

delay(m, s)
char *s;
{

	if(m =& 3)
		printf("%s%d ", s, m);
}


prspeed(c, s)
{
	printf("%s %s baud\n", c, speeds[s].string);
}

printf( s , a )
  register char *s;
{
	register c;
	register *p = &a;

  while ( c = *s++ )  {
	if ( c == '%' )  switch ( c = *s++ )  {
		case 's':	printf( *p++ );  continue;
		case 'c':	c = *p++;  break;
		case 'd':	printn( *p++ );
				continue;
		case '\0':	return;
	}
	putc( c );
  }
}


printn( n )
  register n;
{
	register d;

  if ( d = n/10 )
	printn( d );
  putc( n%10 + '0' );
}


putc(c)
{

	write(2, &c, 1);
}
