#define BYTE 8
#define PAIR(A,B) (A|(B<<BYTE))
int chtab [] = {
		/* printable special characters */
PAIR('q','o'),	0042,	/* " */
PAIR('s','h'),	0043,	/* # */
PAIR('d','o'),	0044,	/* $ */
PAIR('a','a'),	0047,	/* ' */
PAIR('f','m'),	0047,	/* ' */
PAIR('p','r'),	0047,	/* ' */
PAIR('t','s'),	0200,	/* null- terminal sigma not avail. */
PAIR('s','t'),	0052,	/* * */
PAIR('*','*'), 0052,	/*math * */
PAIR('p','l'),	0053,	/* + */
PAIR('e','q'),	0075,	/* = */
PAIR('m','i'),	0055,	/* - */
PAIR('h','y'),	0055,	/* hyphen */
PAIR('s','l'),	0057,	/* / */
PAIR('a','t'),	0100,	/* @ */
PAIR('b','a'),	0207,	/* \ */
PAIR('u','l'),	0137,	/* _ */
PAIR('r','u'),	0137,	/* rule */
PAIR('a','c'),	0140,	/* ` */
PAIR('g','a'),	0140,	/* ` */
PAIR('b','b'),	0174,	/* | */
PAIR('t','l'),	0176,	/* ~ */
		/* nonprinting ascii characters */
PAIR('d','l'),	0177,	/* delete */
PAIR('n','u'),	0200,	/* null */
PAIR('b','s'),	0201,	/* back space */
PAIR('h','t'),	0202,	/* horizontal tab */
PAIR('f','l'),	0203,	/* linefeed */
PAIR('f','f'),	0204,	/* formfeed */
PAIR('c','r'),	0205,	/* carriage return */
PAIR('e','s'),	0206,	/* esc */
		/* sanders control sequences */
PAIR('#','a'),	0212,	/* select font */
PAIR('#','s'),	0213,	/* repeat character */
PAIR('#','x'),	0214,	/* insert sequence of characters */
PAIR('#','y'),	0215,	/* insert space */
PAIR('#','U'),	0216,	/* half-line feed */
PAIR('#','F'),	0217,	/* negative line feed */
PAIR('#','D'),	0220,	/* negative line feed */
PAIR('#','n'),	0221,	/* select format */
PAIR('#','e'),	0222,	/* set line length */
PAIR('#','f'),	0223,	/* set left margin */
PAIR('#','g'),	0224,	/* set indentation */
PAIR('#','i'),	0226,	/* set form length */
PAIR('#','t'),	0227,	/* toggle draft mode */
	/* greek: */
PAIR('*','a'), 0230,	/*alpha*/
PAIR('*','b'), 0231,	/*beta*/
PAIR('*','g'), 0232,	/*gamma*/
PAIR('*','d'), 0233,	/*delta*/
PAIR('*','e'), 0234,	/*epsilon*/
PAIR('*','z'), 0235,	/*zeta*/
PAIR('*','y'), 0236,	/*eta*/
PAIR('*','h'), 0237,	/*theta*/
PAIR('*','i'), 0240,	/*iota*/
PAIR('*','k'), 0241,	/*kappa*/
PAIR('*','l'), 0242,	/*lambda*/
PAIR('*','m'), 0243,	/*mu*/
PAIR('*','n'), 0244,	/*nu*/
PAIR('*','c'), 0245,	/*xi*/
PAIR('*','o'), 0246,	/*omicron*/
PAIR('*','p'), 0247,	/*pi*/
PAIR('*','r'), 0250,	/*rho*/
PAIR('*','s'), 0251,	/*sigma*/
PAIR('*','t'), 0252,	/*tau*/
PAIR('*','u'), 0253,	/*upsilon*/
PAIR('*','f'), 0254,	/*phi*/
PAIR('*','x'), 0255,	/*chi*/
PAIR('*','q'), 0256,	/*psi*/
PAIR('*','w'), 0257,	/*omega*/
PAIR('*','A'), 0260,	/*Alpha*/
PAIR('*','B'), 0261,	/*Beta*/
PAIR('*','G'), 0262,	/*Gamma*/
PAIR('*','D'), 0263,	/*Delta*/
PAIR('*','E'), 0264,	/*Epsilon*/
PAIR('*','Z'), 0265,	/*Zeta*/
PAIR('*','Y'), 0266,	/*Eta*/
PAIR('*','H'), 0267,	/*Theta*/
PAIR('*','I'), 0270,	/*Iota*/
PAIR('*','K'), 0271,	/*Kappa*/
PAIR('*','L'), 0272,	/*Lambda*/
PAIR('*','M'), 0273,	/*Mu*/
PAIR('*','N'), 0274,	/*Nu*/
PAIR('*','C'), 0275,	/*Xi*/
PAIR('*','O'), 0276,	/*Omicron*/
PAIR('*','P'), 0277,	/*Pi*/
PAIR('*','R'), 0300,	/*Rho*/
PAIR('*','S'), 0301,	/*Sigma*/
PAIR('*','T'), 0302,	/*Tau*/
PAIR('*','U'), 0303,	/*Upsilon*/
PAIR('*','F'), 0304,	/*Phi*/
PAIR('*','X'), 0305,	/*Chi*/
PAIR('*','Q'), 0306,	/*Psi*/
PAIR('*','W'), 0307,	/*Omega*/
PAIR('s','r'), 0310,	/*square root*/
PAIR('r','n'), 0311,	/*root en*/
PAIR('>','='), 0312,	/*>=*/
PAIR('<','='), 0313,	/*<=*/
PAIR('=','='), 0314,	/*identically equal*/
PAIR('~','='), 0315,	/*approx =*/
PAIR('a','p'), 0316,	/*approximates*/
PAIR('!','='), 0317,	/*not equal*/
PAIR('-','>'), 0320,	/*right arrow*/
PAIR('<','-'), 0321,	/*left arrow*/
PAIR('u','a'), 0322,	/*up arrow*/
PAIR('d','a'), 0323,	/*down arrow*/
PAIR('m','u'), 0324,	/*multiply*/
PAIR('d','i'), 0325,	/*divide*/
PAIR('+','-'), 0326,	/*plus-minus*/
PAIR('c','u'), 0327,	/*cup (union)*/
PAIR('c','a'), 0330,	/*cap (intersection)*/
PAIR('s','b'), 0331,	/*subset of*/
PAIR('s','p'), 0332,	/*superset of*/
PAIR('i','b'), 0333,	/*improper subset*/
PAIR('i','p'), 0334,	/*  " superset*/
PAIR('i','f'), 0335,	/*infinity*/
PAIR('p','d'), 0336,	/*partial derivative*/
PAIR('g','r'), 0337,	/*gradient*/
PAIR('p','t'), 0340,	/*proportional to*/
PAIR('b','r'), 0341,	/*box vert rule*/
PAIR('o','r'), 0342,	/*or*/
PAIR('c','i'), 0276,	/*circle*/
PAIR('b','u'),  0343,	/* bullet */
PAIR('s','q'),  0344,	/* square */
PAIR('d','e'),  0345,	/* degree */
PAIR('d','g'),  0346,	/* dagger */
PAIR('d','d'),  0371,	/* dbl dagger */
		/* stan, various segments, see also bracket building */
PAIR('u','r'),	0347,	/* underrule */
PAIR('e','m'),	0350,	/* 3/4-em dash */
		/* bracket building characters, stan */
PAIR('b','v'),	0351,	/* vertical bar */
PAIR('l','t'),	0352,	/* left top curly */
PAIR('l','b'),	0353,	/* left bottom curly */
PAIR('r','k'),	0354,	/* right center curly */
PAIR('l','c'),	0355,	/* left ceiling */
PAIR('l','f'),	0356,	/* left floor */
PAIR('r','f'),	0357,	/* right floor */
PAIR('r','t'),	0360,	/* right top curly */
PAIR('r','b'),	0361,	/* right bottom curly */
PAIR('r','c'),	0362,	/* right ceiling */
PAIR('l','k'),	0363,	/* left center curly */
PAIR('t','i'),	0364,	/* top of integral */
PAIR('b','i'),	0365,	/* bottom of integral */
PAIR('b','t'),	0366,	/* bracket upper */
PAIR('b','l'),	0367,	/* bracket lower */
PAIR('s','s'),	0370,	/* shifted slash */
PAIR('v','b'),	0372,	/* solid single bar */
0,0};
