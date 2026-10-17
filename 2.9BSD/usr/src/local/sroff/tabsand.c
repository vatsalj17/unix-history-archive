/*
 * sroff driving tables
 */
#include "sdef.h"

/*
 * struct t is not overlaid by -F options.
 */

struct {
#ifndef TTY
	int bflags;
	int bspeed;
#else
	int bset;
	int breset;
#endif
	int Newline;
	int Halfline;
	char *twinit;
	char *twrest;
	char *twnl;
#ifdef ADJ
	char *twfp;
#endif
	char *hlr;
	char *hlf;
	char *flr;
	char *bdon;
	char *bdoff;
	char *ploton;
	char *plotoff;
	char *up;
	char *down;
	char *right;
	char *left;
	char *ulon;     /* underlining enables */
	char *uloff;    /* underlining disabled */
	int zzz;
	} t {

#ifndef TTY
#ifdef ADJ
/*bflags*/	CR1|NL1|FF1|EVENP|ODDP,
#else
/*bflags*/	CR1|NL1|FF1|EVENP|ODDP|CRMOD,
#endif
/*bspeed*/	BSPEED,

#else

/*bset*/	CR1|NL1|FF1|EVENP|ODDP,
#ifdef ADJ
/*breset*/	ECHO|CRMOD,
#else
/*breset*/	ECHO,
#endif
#endif	TTY

/*Newline*/	INCH/6,
/*Halfline*/	INCH/12,
/*twinit*/  "\033I\033rA\033B@@A", /* reset, ribbon usage, bottom margin */
/*twrest*/  "",
#ifdef ADJ
/*twnl*/    "\r\n",
/*twfp*/    "\033P\n",	/* force print */
#else
/*twnl*/    "\n",
#endif
/*hlr*/     "\033D",
/*hlf*/     "\033U",
/*flr*/     "\033\012",    /*reverse line feed for col*/
/*bdon*/	"\033b",	/* bold on - next char is offset */
/*bdoff*/	"\033b@",	/* try to turn it back off!! */
/*ploton*/	"",
/*plotoff*/	"",
/*up*/		"",
/*down*/	"",
/*right*/	"",
/*left*/	"",
/*ulon*/    "\033uM",
/*uloff*/   "\033uM"
};

/*
 * code tables
 * In the first byte, the
 * 0100 bit specifies that special fonts are used; if set, the rightmost octal
 * digit is the font to be set initially, and the middle octal digit
 * is the final font that will be used (always the same if no font changes
 * are done in the string).
 * The leftmost bit of each of the remaining bytes
 * turns on plot mode (see s10.c)
 */

char *codetab[MAXCHAR-0200] = {
"\000\000",		/* 200: null; not output */
"\000\010",		/* 201: bs */
"\000\011",		/* 202: tab */
"\000\012",		/* 203: lf */
"\000\014",		/* 204: ff */
"\000\015",		/* 205: cr */
"\000\033",		/* 206: esc */
"\000\\",		/* backslash (never escaped) */
"\000\000",		/* unused */
"\000\000",		/* unused */
			/* sanders control sequences */
"\000\033a",		/* 212: select font */
"\000\033s",		/* 213:  repeat character */
"\000\033x",		/* 214:  insert sequence of characters */
"\000\033y",		/* 215:  insert space */
"\000\033U",		/* 216:  half-line feed */
"\000\033\012",		/* 217:  neqative line feed */
"\000\033D",		/* 220:  negative half-line feed */
"\000\033n",		/* 221:  select format */
"\000\033e",		/* 222:  set line length */
"\000\033I",		/* 223:  set temporary indent */
"\000\033g",		/* 224:  set indentation */
#ifdef ADJ
"\000\033w@@,@@,FP.",	/* 225:  set word space */
			/* allow up to 400 mils between words */
#else
"\000\033w",		/* 225:  set word space */
#endif
"\000\033i",		/* 226:  set form length */
"\000\033t",		/* 227:  toggle draft mode */
				/* greek: */
"\177a",		/* 230: alpha */
"\177b",		/* beta */
"\177c",		/* gamma */
"\177d",		/* delta */
"\177e",		/* epsilon */
"\177g",		/* zeta */
"\177h",		/* eta */
"\177i",		/* theta */
"\177j",		/*240: iota */
"\177k",		/* kappa */
"\177l",		/* lambda */
"\177m",		/* mu */
"\177n",		/* nu */
"\177o",		/* xi */
"\177p",		/* omicron */
"\177q",		/* pi */
"\177r",		/*250: rho */
"\177s",		/* sigma */
"\177t",		/* tau */
"\177u",		/* upsilon */
"\177v",		/* phi */
"\177x",		/* chi */
"\177y",		/* psi */
"\177z",		/*omega*/
"\177A",		/*260: Alpha */
"\177B",		/* Beta */
"\177C",		/* Gamma */
"\177D",		/* Delta */
"\177E",		/* Epsilon */
"\177G",		/* Zeta */
"\177H",		/* Eta */
"\177I",		/* Theta */
"\177J",		/*270: Iota */
"\177K",		/* Kappa */
"\177L",		/* Lambda */
"\177M",		/* Mu */
"\177N",		/* Nu */
"\177O",		/* Xi */
"\177P",		/* Omicron */
"\177Q",		/* Pi */
"\177R",		/*300: Rho */
"\177S",		/* Sigma */
"\177T",		/* Tau */
"\177U",		/* Upsilon */
"\177V",		/* Phi */
"\177X",		/* Chi */
"\177Y",		/* Psi */
"\177Z",		/* Omega */
"\177`",		/*310: square root*/
"\177_",		/*root en */
"\166h",		/*>=*/
"\166H",		/*<=*/
"\166N",		/*identically equal*/
"\166k",		/*approx =*/
"\166J",		/*approximates*/
"\166j",		/*not equal*/
"\166W",		/*320: right arrow*/
"\166w",		/*left arrow*/
"\166^",		/*up arrow*/
"\166\177",		/*down arrow*/
"\166x",		/*multiply*/
"\166f",		/*divide*/
"\166L",		/*plus-minus*/
"\166T",		/*cup (union)*/
"\166t",		/*330: cap (intersection)*/
"\166U",		/*subset of*/
"\166u",		/*superset of*/
"\166\033uDU\033uD",	/*improper subset*/
"\166\033uDu\033uD",	/* improper superset*/
"\166z",		/*infinity*/
"\166X",		/*partial derivative*/
"\166V",	 	/*gradient*/
"\166Z",		/*340: proportional to*/
"\166\033l\177Nd\033o\177p\033l~\134d\033o@P\033l\177N",   /* box vert rule */
"\177|",		 /*or */
"\000*",		/*bullet*/
"\166[",		/*square*/
"\177;",		/*degree*/
"\166_",		/*dagger*/
"\000\033o@K\_\033o\177u",	/* underrule */
"\177-",		/* 350: 3/4 Em dash */
        				/* bracket building symbols */
"\166\033o@Dd\033o\177l\033l~\134d\033o@P",	/* vertical solid bar */
"\166A\033o@N\033l~\134d\033o\177r",		/* left top curly */
"\166B\033o\177t\033l~\134d\033o@L",		/* left bottom curly */
"\166C\033o\177r\033l~\134d\033o@N",		/* right center curly */
"\166D\033o@N\033l~\134d\033o\177r",		/* left top square */
"\166E\033o\177t\033l~\134d\033o@L",		/* left bottom square */
"\166F\033o\177t\033l~\134d\033o@L",		/* right bottom square */
"\166b\033o@N\033l~\134d\033o\177r",		/*360: right top curly */
"\166c\033o\177t\033l~\134d\033o@L",		/* right bottom curly */
"\166e\033o@N\033l~\134d\033o\177r",		/* right top square */
"\166/\033o\177r\033l~\134d\033o@N",		/* left center curly */
"\167!\033aF\033o@N\033l~\134d\033o\177r",	/* top of integral */
"\176\033o\177rd\033o@N\033l~\134\033aG*",	/* bottom of integral */
"\166d\033o\177r\033l~\134d\033o@N",		/* bracket upper */
"\166d\033o\177r\033l~\134d\033o@N",		/* bracket lower */
"\177\\",				/* 370: angled slash for sqrt */
					/* more constructed characters: */
"\177|\033N=",				/*371: dbl dagger*/
"\166d"					/* solid single bar */
};
