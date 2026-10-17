#
/*
 * sed -- stream  editor
 *
 * Copyright 1975 Bell Telephone Laboratories, Incorporated
 *
 * Owner: lem
 */

#define CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	CNL	8
#define	CDOL	10
#define	CEOF	11
#define CKET	12
#define CNULL	13
#define CLNUM	14
#define CEND	16
#define CDONT	17
#define	CBACK	18

#define	STAR	01

#define CGMES	"command garbled: %s\n", linebuf
#define TMMES	"Too much text: %s\n", linebuf
#define LTL	"Label too long: %s\n", linebuf
#define AD0MES "No addresses allowed: %s\n", linebuf
#define AD1MES "Only one address allowed: %s\n", linebuf
#define NLINES	256
#define	DEPTH	20
#define PTRSIZE	100
#define RESIZE	5000
#define	ABUFSIZE	20
#define	LBSIZE	512
#define	ESIZE	256
#define	LABSIZE	50
#define NBRA	9

char	*abuf[ABUFSIZE];
char	**aptr;
char	*lastre;
char	ibuf[512];
char	*cbp;
char	*ebp;
char	genbuf[LBSIZE];
char	*loc1;
char	*loc2;
char	*locs;
char	seof;
char	*reend;
char	*lbend;
struct reptr	*ptrend;
char	eflag;
int	dolflag;
int	sflag;
int	jflag;
int	numbra;
int	delflag;
long	lnum;
char	linebuf[LBSIZE+1];
char	*spend;
int	nflag;
int	gflag;
char	*braelist[NBRA];
char	*braslist[NBRA];
long	tlno[NLINES];
int	nlno;
char	fname[12][40];
int	fcode[12];
int	nfiles;

#define ACOM	4
#define CCOM	8
#define	CDCOM	84
#define	CNCOM	72
#define COCOM	60
#define	CPCOM	76
#define DCOM	12
#define ECOM	52
#define EQCOM	44
#define FCOM	56
#define ICOM	16
#define JCOM	64
#define LCOM	20
#define NCOM	40
#define PCOM	32
#define QCOM	36
#define RCOM	24
#define SCOM	28
#define TCOM	68
#define WCOM	48
#define	CWCOM	80
#define GCOM	1
#define VCOM 	2

char	*cp;
char	*reend;
char	*lbend;

struct reptr {
	char	*ad1;
	char	*ad2;
	char	*re1;
	char	*re2;
	char	*rhs;
	int	fcode;
	char	command;
	char	gfl;
	char	pfl;
	char	inar;
	char	nlf;
} ptrspace[PTRSIZE];

struct reptr	*rep;

char	respace[RESIZE];

struct label {
	char	asc[9];
	struct reptr	*chain;
	struct reptr	*address;
} labtab[LABSIZE];

struct label	*lab;
struct label	*labend;

int	f;
int	snlf;
int	depth;

int	eargc;
char	**eargv;

extern	char	bittab[];

int	*cmpend[DEPTH];
int	depth;
struct reptr	*pending;
