#include <signal.h>
#include <sgtty.h>

	/* printer parameters */
#define BSPEED	B2400
#define	HOR	1	/* 1 mil */
#define	VERT	3	/* actually 3.5 mil */
#define	PTID	0
#define	FT	0	/*default font position*/
#define PFONT	3	/* maximum number of proportional fonts in -F option */
	/*
	 * default spacings
	 */
#define INCH	1000		/*increments per inch*/
#define stou(x)	(int)((long)(x)*7/2)	/* internal vertical steps to units */
#ifdef PGOFF
#define PO	INCH 		/*page offset*/
#else
#define PO	0
#endif
#define DTAB	INCH/2		/* tab stops */
#define NDTAB	INCH/10*8	/* tabs for -nroff */
#define LL	(6*INCH+INCH/2)	/*line length; 39picas=6.5in*/
#define SD_LL	(7*INCH)	/* printer default line length */
#define TOP_MARG   INCH/2	/* printer (hard) top margin */
#define VS	INCH/6		/*vert space; 12points*/

#define LG	0

	/*
	 * internal characters
	 */
#define	LEADER	001
#define	IMP	004		/*impossible char*/
#define	TAB	011
#define	COLON	013
#define	RPT	014
#define	OHC	024
#define	CONT	025
#define	PRESC	026
#define	UNPAD	027
#define	XPAR	030
#define	FLSS	031
#define	TDELIM	032
#define	ESC	033
#define	LEFT	035
#define	RIGHT	036
#define	FILLER	037
#define	JREG	0374
#define	LX	0375		/*low-order part of xlss*/
#define	HX	0376		/*High-order part of xlss*/


	/*
	 * output characters
	 */

	/* ascii control characters */
#define C_DEL	0177		/* DEL, 0177 */
#define C_NUL	0200		/* NULL */
#define C_BS	0201		/* backspace */
#define C_TAB	0202		/* horizontal tab */
#define C_LF	0203		/* linefeed */
#define C_FF	0204		/* formfeed */
#define C_CR	0205		/* carriage return */
#define C_ESC	0206		/* ESC, 033 */
	/* sanders escape sequences */
#define S_FT	0212		/* select font */
#define S_AD	0221		/* select format (adjust) */
#define S_LL	0222		/* set line length */
#define S_TI	0223		/* set temporary indent */
#define S_IN	0224		/* set indent */
#define S_WS	0225		/* set word spacing */
#define S_PL	0226		/* set page length */
	/* characters receiving special handling */
#define C_HY	055		/* hyphen */
#define C_RN	0311		/* root en */
#define C_BR	0341		/* vertical box rule */
#define C_VB	0372		/* single vertical bar */
#define C_UR	0347		/* underrule */
#define C_EM	0350		/* 3/4 em dash */
#define MAXCHAR 0373	/* 1 greater than max printing character */

/*
 *  Sanders escape sequences used internally (sent directly)
 */
#define SE_CHFONT	"\033a"		/* change font */
#define SE_MOUNT	"\033d"		/* mount font */
#define SE_DRAFT	"\033t"		/* mount font */
#define SE_LMARGIN	"\033f"		/* set page offset */

#define SE_VMOT		"\033o"		/* vertical motion */
#define SE_VBLOCK	2047		/* chunks for vertical motion */
#define SE_UPBL		"\033o`A"	/* up VBLOCK */
#define SE_DOWNBL	"\033o_\177"	/* down VBLOCK */
#define SE_UP1		"\033o\177\177"	/* up 1 step */
#define SE_DOWN1	"\033o@A"	/* down 1 step */

#define SE_HMOT		"\033l"		/* horizontal motion */
#define SE_HBLOCK	2047		/* chunks for horizontal motion */
#define SE_LEFTBL	"\033l`A"	/* left HBLOCK */
#define SE_RIGHTBL	"\033l_\177"	/* right HBLOCK */


#define ZONE 8	/*8hrs for PST*/
	/*
	 * fonts; bits in chbits
	 */
#define ZBIT   010000	/*zero width char*/
#define ULMODE 020000	/* underline this character */
#define EBIT   040000	/* stan, character in escape sequence for sanders */
#define NFONTS 8		/* power of 2, maximum 16 */
#define FONTSHFT 8		/* position of font bits in chbits */
#define FONTMSK (NFONTS-1)	/* physical font in fonts[] */
#define PROPOR 0100		/* physical font is proportional */
#define getfont(i)	((i>>FONTSHFT)&FONTMSK)	/* font of char i */

	/* more bit masks */
#define DBL	0100000		/*double size indicator*/
#define MOT	0100000		/*motion character indicator*/
#define MOTV	0160000		/*clear for motion part*/
#define VMOT	0040000		/*vert motion bit*/
#define NMOT	0020000		/* negative motion indicator*/
#define MMASK	0100000		/*macro mask indicator*/
#define CMASK	0100377
#define WDMASK	01777		/* width part of *fontab */
#define BMASK	0377
#define BYTE	8
#define TMASK	037777
#define RTAB	0100000
#define CTAB	0040000


#define PAIR(A,B) (A|(B<<BYTE))

	/* array sizes and limits */
#define NN	200	/*number registers*/
/* NN changed Jan 31 from 132 */
#define NNAMES	14	/*predefined reg names*/
#define NIF	15	/*if-else nesting*/
#define NEV	3	/*environments*/
#define EVLSZ	10	/*size of ev stack*/
#define EVS	4*256	/*environment size in words*/
/* BWK - trying 4*256 instead of 3*256 */
#ifdef VMUNIX
#define NM	500	/*requests + macros */
#else
#define NM	300	/*requests + macros*/
#endif
#define NHYP	10	/*max hyphens per word*/
#define NHEX	128	/*byte size of exception word list*/
#define NTAB	35	/*tab stops*/
#define NSO	5	/*"so" depth*/
#define NDI	5	/*number of diversions*/
#define NPN	20	/*numbers in "-o"*/
#define NPP	10	/*pads per field*/
#define NOV	10	/*number of overstrike chars*/
#define NTRAP	20	/*number of traps*/

	/* buffer sizes */
#define NS	64	/*name buffer*/
#define NTM	256	/*tm buffer*/
#define DELTA	512	/*delta core bytes*/
#define WDSIZE	200	/*stan, changed from 170, word buffer size*/
#define LNSIZE	680	/*line buffer size*/
/* BWK - changed from 480 after EVS changed */
#define FBUFSZ	256	/*field buf size words*/
#define OBUFSZ	512	/*bytes*/
#define IBUFSZ	512	/*bytes*/
#define NC	256	/*cbuf size words*/
#ifdef	VMUNIX
#define MAXPTR (char *)0x7fffffff  /* max value of any pointer variable */
#else
#define	MAXPTR	(char *)(-1)	/* max value of any pointer variable */
#endif

	/* temp file management */
#define BLK	128	/*alloc block words*/
#define TEMP	512	/*65Kw*/
#ifdef BIG
typedef long filep;
#define NBLIST	BIG	/*allocation , BIG = 256 per 65k*/
#define BLKBITS 7	/*for BLK=128*/
#endif
#ifndef BIG
typedef unsigned filep;
#define NBLIST	TEMP	/*allocation list, TEMP<=512*/
/* BLK*NBLIST<=65536 words, if filep=unsigned */
#define BLKBITS 0
#endif
