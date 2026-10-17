/*
character name and code tables
default width tables
modified for BTL special font version 4
and Commercial II
Tuned for Versatec 1200A -- pag
*/

int chtab [] {
'hy', 0200,	/*hyphen*/
'bu', 0201,	/*bullet*/
'sq', 0202,	/*square*/
'em', 0203,	/*3/4em*/
'ru', 0204,	/*rule*/
'14', 0205,	/*1/4*/
'12', 0206,	/*1/2*/
'34', 0207,	/*3/4*/
'mi', 0302,	/*equation minus*/
'fi', 0211,	/*fi*/
'fl', 0212,	/*fl*/
'ff', 0213,	/*ff*/
'Fi', 0214,	/*ffi*/
'Fl', 0215,	/*ffl*/
'de', 0216,	/*degree*/
'dg', 0217,	/*dagger*/
'sc', 0220,	/*section*/
'fm', 0221,	/*foot mark*/
'aa', 0222,	/*acute accent*/
'ga', 0223,	/*grave accent*/
'ul', 0224,	/*underrule*/
'sl', 0225,	/*slash (longer)*/
'*a', 0230,	/*alpha*/
'*b', 0231,	/*beta*/
'*g', 0232,	/*gamma*/
'*d', 0233,	/*delta*/
'*e', 0234,	/*epsilon*/
'*z', 0235,	/*zeta*/
'*y', 0236,	/*eta*/
'*h', 0237,	/*theta*/
'*i', 0240,	/*iota*/
'*k', 0241,	/*kappa*/
'*l', 0242,	/*lambda*/
'*m', 0243,	/*mu*/
'*n', 0244,	/*nu*/
'*c', 0245,	/*xi*/
'*o', 0246,	/*omicron*/
'*p', 0247,	/*pi*/
'*r', 0250,	/*rho*/
'*s', 0251,	/*sigma*/
'*t', 0252,	/*tau*/
'*u', 0253,	/*upsilon*/
'*f', 0254,	/*phi*/
'*x', 0255,	/*chi*/
'*q', 0256,	/*psi*/
'*w', 0257,	/*omega*/
'*A', 0101,	/*Alpha*/
'*B', 0102,	/*Beta*/
'*G', 0260,	/*Gamma*/
'*D', 0261,	/*Delta*/
'*E', 0105,	/*Epsilon*/
'*Z', 0132,	/*Zeta*/
'*Y', 0110,	/*Eta*/
'*H', 0262,	/*Theta*/
'*I', 0111,	/*Iota*/
'*K', 0113,	/*Kappa*/
'*L', 0263,	/*Lambda*/
'*M', 0115,	/*Mu*/
'*N', 0116,	/*Nu*/
'*C', 0264,	/*Xi*/
'*O', 0117,	/*Omicron*/
'*P', 0265,	/*Pi*/
'*R', 0120,	/*Rho*/
'*S', 0266,	/*Sigma*/
'*T', 0124,	/*Tau*/
'*U', 0270,	/*Upsilon*/
'*F', 0271,	/*Phi*/
'*X', 0130,	/*Chi*/
'*Q', 0272,	/*Psi*/
'*W', 0273,	/*Omega*/
'sr', 0274,	/*square root*/
'ts', 0275,	/*terminal sigma*/
'rn', 0276,	/*root en*/
'>=', 0277,	/*>=*/
'<=', 0300,	/*<=*/
'==', 0301,	/*identically equal*/
'~=', 0303,	/*approx =*/
'ap', 0304,	/*approximates*/
'!=', 0305,	/*not equal*/
'->', 0306,	/*right arrow*/
'<-', 0307,	/*left arrow*/
'ua', 0310,	/*up arrow*/
'da', 0311,	/*down arrow*/
'eq', 0312,	/*equation equal*/
'mu', 0313,	/*multiply*/
'di', 0314,	/*divide*/
'+-', 0315,	/*plus-minus*/
'cu', 0316,	/*cup (union)*/
'ca', 0317,	/*cap (intersection)*/
'sb', 0320,	/*subset of*/
'sp', 0321,	/*superset of*/
'ib', 0322,	/*improper subset*/
'ip', 0323,	/*  " superset*/
'if', 0324,	/*infinity*/
'pd', 0325,	/*partial derivative*/
'gr', 0326,	/*gradient*/
'no', 0327,	/*not*/
'is', 0330,	/*integral sign*/
'pt', 0331,	/*proportional to*/
'es', 0332,	/*empty set*/
'ag', 0333,	/*angstrom*/
'pl', 0334,	/*equation plus*/
'rg', 0335,	/*registered*/
'co', 0336,	/*copyright*/
'br', 0337,	/*box vert rule*/
'ct', 0340,	/*cent sign*/
'dd', 0341,	/*dbl dagger*/
'rh', 0342,	/*right hand*/
'lh', 0343,	/*left hand*/
'**', 0344,	/*math * */
'bs', 0345,	/*bell system sign*/
'or', 0346,	/*or*/
'ci', 0347,	/*circle*/
'lt', 0350,	/*left top (of big curly)*/
'lb', 0351,	/*left bottom*/
'rt', 0352,	/*right top*/
'rb', 0353,	/*right bot*/
'lk', 0354,	/*left center of big curly bracket*/
'rk', 0355,	/*right center of big curly bracket*/
'bv', 0356,	/*bold vertical*/
'lf', 0357,	/*left floor (left bot of big sq bract)*/
'rf', 0360,	/*right floor (rb of ")*/
'lc', 0361,	/*left ceiling (lt of ")*/
'rc', 0362,	/*right ceiling (rt of ")*/
0,0};

char codetab[256-32] {	/*cat codes*/
00,	/*space*/
0145,	/*!*/
0160,	/*"*/
0161,	/*#*/
0155,	/*$*/
053,	/*%*/
050,	/*&*/
032,	/*' close*/
0132,	/*(*/
0133,	/*)*/
0122,	/***/
0143,	/*+*/
047,	/*,*/
040,	/*- hyphen*/
044,	/*.*/
043,	/*/*/
0110,	/*0*/
0111,	/*1*/
0112,	/*2*/
0113,	/*3*/
0114,	/*4*/
0115,	/*5*/
0116,	/*6*/
0117,	/*7*/
0120,	/*8*/
0121,	/*9*/
0142,	/*:*/
023,	/*;*/
0162,	/*<*/
0140,	/*=*/
0163,	/*>*/
0147,	/*?*/
0164,	/*@*/
0103,	/*A*/
075,	/*B*/
070,	/*C*/
074,	/*D*/
072,	/*E*/
0101,	/*F*/
065,	/*G*/
060,	/*H*/
066,	/*I*/
0105,	/*J*/
0107,	/*K*/
063,	/*L*/
062,	/*M*/
061,	/*N*/
057,	/*O*/
067,	/*P*/
055,	/*Q*/
064,	/*R*/
076,	/*S*/
056,	/*T*/
0106,	/*U*/
071,	/*V*/
0104,	/*W*/
0102,	/*X*/
077,	/*Y*/
073,	/*Z*/
0134,	/*[*/
0165,	/*\*/
0135,	/*]*/
0166,	/*^*/
022,	/*_ dash*/
030,	/*` open*/
025,	/*a*/
012,	/*b*/
027,	/*c*/
011,	/*d*/
031,	/*e*/
014,	/*f*/
045,	/*g*/
001,	/*h*/
006,	/*i*/
015,	/*j*/
017,	/*k*/
005,	/*l*/
004,	/*m*/
003,	/*n*/
033,	/*o*/
021,	/*p*/
042,	/*q*/
035,	/*r*/
010,	/*s*/
002,	/*t*/
016,	/*u*/
037,	/*v*/
041,	/*w*/
013,	/*x*/
051,	/*y*/
007,	/*z*/
0167,	/*{*/
0151,	/*|*/
0170,	/*}*/
0171,	/*~*/
00,	/*narrow space*/
040,	/*hyphen*/
0146,	/*bullet*/
0154,	/*square*/
022,	/*3/4 em*/
026,	/*rule*/
034,	/*1/4*/
036,	/*1/2*/
046,	/*3/4*/
0123,	/*minus*/
0124,	/*fi*/
0125,	/*fl*/
0126,	/*ff*/
0131,	/*ffi*/
0130,	/*ffl*/
0136,	/*degree*/
0137,	/*dagger*/
0355,	/*section*/
0150,	/*foot mark*/
0334,	/*acute accent*/
0335,	/*grave accent*/
0240,	/*underrule*/
0304,	/*slash (longer)*/
00,	/*half nar sp*/
00,	/**/
0225,	/*alpha*/
0212,	/*beta*/
0245,	/*gamma*/
0211,	/*delta*/
0231,	/*epsilon*/
0207,	/*zeta*/
0214,	/*eta*/
0202,	/*theta*/
0206,	/*iota*/
0217,	/*kappa*/
0205,	/*lambda*/
0204,	/*mu*/
0203,	/*nu*/
0213,	/*xi*/
0233,	/*omicron*/
0221,	/*pi*/
0235,	/*rho*/
0210,	/*sigma*/
0237,	/*tau*/
0216,	/*upsilon*/
0215,	/*phi*/
0227,	/*chi*/
0201,	/*psi*/
0251,	/*omega*/
0265,	/*Gamma*/
0274,	/*Delta*/
0256,	/*Theta*/
0263,	/*Lambda*/
0302,	/*Xi*/
0267,	/*Pi*/
0276,	/*Sigma*/
00,	/**/
0306,	/*Upsilon*/
0255,	/*Phi*/
0242,	/*Psi*/
0257,	/*Omega*/
0275,	/*square root*/
0262,	/*terminal sigma (was root em)*/
0261,	/*root en*/
0327,	/*>=*/
0326,	/*<=*/
0330,	/*identically equal*/
0264,	/*equation minus*/
0277,	/*approx =*/
0272,	/*approximates*/
0331,	/*not equal*/
0354,	/*right arrow*/
0234,	/*left arrow*/
0236,	/*up arrow*/
0223,	/*down arrow*/
0232,	/*equation equal*/
0323,	/*multiply*/
0324,	/*divide*/
0325,	/*plus-minus*/
0260,	/*cup (union)*/
0305,	/*cap (intersection)*/
0270,	/*subset of*/
0271,	/*superset of*/
0350,	/*improper subset*/
0246,	/* improper superset*/
0244,	/*infinity*/
0273,	/*partial derivative*/
0253,	/*gradient*/
0307,	/*not*/
0266,	/*integral sign*/
0247,	/*proportional to*/
0343,	/*empty set*/
0341,	/*angstrom*/
0353,	/*equation plus*/
0141,	/*registered*/
0153,	/*copyright*/
0346,	/*box rule (was parallel sign)*/
0127,	/*cent sign*/
0345,	/*dbl dagger*/
0250,	/*right hand*/
0340,	/*left hand*/
0347,	/*math * */
0243,	/*bell system sign*/
0226,	/*or (was star)*/
0351,	/*circle*/
0311,	/*left top (of big curly)*/
0314,	/*left bottom*/
0315,	/*right top*/
0317,	/*right bot*/
0313,	/*left center of big curly bracket*/
0316,	/*right center of big curly bracket*/
0312,	/*bold vertical*/
0321,	/*left floor (left bot of big sq bract)*/
0320,	/*right floor (rb of ")*/
0322,	/*left ceiling (lt of ")*/
0310};	/*right ceiling (rt of ")*/

/*modified for Commercial II*/
/*  Graphic Wonder Roman Font  */
char W1[256-32] {
12,		/* space */
12,		/* ! */
18,		/* " */
24,		/* # */
22,		/* $ */
27,		/* % */
28,		/* & */
12,		/* ' */
16,		/* ( */
16,		/* ) */
19,		/* * */
28,		/* + */
12,		/* , */
14,		/* - hyphen */
12,		/* . */
24,		/* / */
22+0200,	/* 0 */
22+0200,	/* 1 */
22+0200,	/* 2 */
22+0200,	/* 3 */
22+0200,	/* 4 */
22+0200,	/* 5 */
22+0200,	/* 6 */
22+0200,	/* 7 */
22+0200,	/* 8 */
22+0200,	/* 9 */
12,		/* : */
12,		/* ; */
27,		/* < */
28,		/* = */
27,		/* > */
20,		/* ? */
30,		/* @ */
24+0200,	/* A */
26+0200,	/* B */
25+0200,	/* C */
26+0200,	/* D */
25+0200,	/* E */
24+0200,	/* F */
27+0200,	/* G */
29+0200,	/* H */
13+0200,	/* I */
18+0200,	/* J */
26+0200,	/* K */
22+0200,	/* L */
30+0200,	/* M */
27+0200,	/* N */
26+0200,	/* O */
26+0200,	/* P */
26+0300,	/* Q */
26+0200,	/* R */
24+0200,	/* S */
22+0200,	/* T */
29+0200,	/* U */
24+0200,	/* V */
29+0200,	/* W */
24+0200,	/* X */
25+0200,	/* Y */
24+0200,	/* Z */
16,		/* [ */
24,		/* \ */
16,		/* ] */
24,		/* ^ */
28,		/* _ */
12,		/* ` */
22,		/* a */
24+0200,	/* b */
22,		/* c */
24+0200,	/* d */
22,		/* e */
14+0200,	/* f */
22+0100,	/* g */
24+0200,	/* h */
12+0200,	/* i */
12+0300,	/* j */
24+0200,	/* k */
12+0200,	/* l */
37,		/* m */
24,		/* n */
22,		/* o */
24+0100,	/* p */
22+0100,	/* q */
19,		/* r */
19,		/* s */
17+0200,	/* t */
24,		/* u */
20,		/* v */
27,		/* w */
22,		/* x */
22+0100,	/* y */
20,		/* z */
16,		/* { */
9,		/* | */
16,		/* } */
27,		/* ~ */
6,		/* narrow space */
14,		/* hyphen */
9,		/* bullet */
9,		/* square */
28,		/* 3/4 em dash */
14,		/* rule */
24,		/* 1/4 */
24,		/* 1/2 */
24,		/* 3/4 */
28,		/* minus */
24,		/* fi */
24,		/* fl */
26,		/* ff */
37,		/* ffi */
37,		/* ffl */
16,		/* degree */
18,		/* dagger */
0,		/* section (unimplem) */
9,		/* foot mark */
0,		/* acute acc (unimplem) */
0,		/* grave acc (unimplem) */
0,		/* underrule (unimplem) */
0,		/* slash (unimplem) */
3,		/* half narrow space */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
22,		/* registered */
22,		/* copyright */
0,		/* null */
22,		/* cent */
};

/*	Graphic Wonder Italic font  */
char W2[256-32] {
12,		/* space */
12,		/* ! */
20,		/* " */
24,		/* # */
24,		/* $ */
27,		/* % */
30,		/* & */
12,		/* ' */
17,		/* ( */
17,		/* ) */
19,		/* * */
28,		/* + */
12,		/* , */
14,		/* - hyphen */
12,		/* . */
24,		/* / */
24+0200,	/* 0 */
24+0200,	/* 1 */
24+0200,	/* 2 */
24+0200,	/* 3 */
24+0200,	/* 4 */
24+0200,	/* 5 */
24+0200,	/* 6 */
24+0200,	/* 7 */
24+0200,	/* 8 */
24+0200,	/* 9 */
12,		/* : */
12,		/* ; */
27,		/* < */
28,		/* = */
27,		/* > */
24,		/* ? */
30,		/* @ */
24+0200,	/* A */
29+0200,	/* B */
25+0200,	/* C */
27+0200,	/* D */
27+0200,	/* E */
26+0200,	/* F */
26+0200,	/* G */
31+0200,	/* H */
16+0200,	/* I */
22+0200,	/* J */
27+0200,	/* K */
24+0200,	/* L */
32+0200,	/* M */
30+0200,	/* N */
26+0200,	/* O */
27+0200,	/* P */
26+0300,	/* Q */
29+0200,	/* R */
27+0200,	/* S */
25+0200,	/* T */
30+0200,	/* U */
24+0200,	/* V */
31+0200,	/* W */
26+0200,	/* X */
25+0200,	/* Y */
26+0200,	/* Z */
16,		/* [ */
24,		/* \ */
16,		/* ] */
24,		/* ^ */
28,		/* _ */
12,		/* ` */
24,		/* a */
22+0200,	/* b */
20,		/* c */
24+0200,	/* d */
20,		/* e */
18+0300,	/* f */
22+0100,	/* g */
24+0200,	/* h */
14+0200,	/* i */
14+0300,	/* j */
22+0200,	/* k */
14+0200,	/* l */
37,		/* m */
26,		/* n */
20,		/* o */
24+0100,	/* p */
22+0100,	/* q */
19,		/* r */
19,		/* s */
16+0200,	/* t */
26,		/* u */
22,		/* v */
32,		/* w */
22,		/* x */
24+0100,	/* y */
22,		/* z */
16,		/* { */
9,		/* | */
16,		/* } */
27,		/* ~ */
6,		/* narrow space */
14,		/* hyphen */
9,		/* bullet */
9,		/* square */
28,		/* 3/4 em dash */
14,		/* rule */
0,		/* 1/4 */
0,		/* 1/2 */
0,		/* 3/4 */
28,		/* minus */
27,		/* fi */
27,		/* fl */
30,		/* ff */
38,		/* ffi */
38,		/* ffl */
16,		/* degree */
18,		/* dagger */
0,		/* section (unimplem) */
9,		/* foot mark */
0,		/* acute acc (unimplem) */
0,		/* grave acc (unimplem) */
0,		/* underrule (unimplem) */
0,		/* slash (unimplem) */
3,		/* half narrow space */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* registered */
0,		/* copyright */
0,		/* null */
0,		/* cent */
};

/*  Graphic Wonder Triplex Font  */
char W3[256-32] {
12,		/* space */
12,		/* ! */
20,		/* " */
24,		/* # */
22,		/* $ */
27,		/* % */
30,		/* & */
12,		/* ' */
16,		/* ( */
16,		/* ) */
18,		/* * */
28,		/* + */
12,		/* , */
14,		/* - hyphen */
12,		/* . */
26,		/* / */
22+0200,	/* 0 */
22+0200,	/* 1 */
22+0200,	/* 2 */
22+0200,	/* 3 */
22+0200,	/* 4 */
22+0200,	/* 5 */
22+0200,	/* 6 */
22+0200,	/* 7 */
22+0200,	/* 8 */
22+0200,	/* 9 */
12,		/* : */
12,		/* ; */
27,		/* < */
28,		/* = */
27,		/* > */
22,		/* ? */
30,		/* @ */
24+0200,	/* A */
26+0200,	/* B */
25+0200,	/* C */
26+0200,	/* D */
25+0200,	/* E */
24+0200,	/* F */
27+0200,	/* G */
29+0200,	/* H */
14+0200,	/* I */
19+0200,	/* J */
26+0200,	/* K */
22+0200,	/* L */
31+0200,	/* M */
29+0200,	/* N */
26+0200,	/* O */
26+0200,	/* P */
26+0300,	/* Q */
26+0200,	/* R */
24+0200,	/* S */
24+0200,	/* T */
29+0200,	/* U */
24+0200,	/* V */
29+0200,	/* W */
24+0200,	/* X */
26+0200,	/* Y */
24+0200,	/* Z */
16,		/* [ */
19,		/* \ */
16,		/* ] */
24,		/* ^ */
28,		/* _ */
12,		/* ` */
22,		/* a */
24+0200,	/* b */
22,		/* c */
24+0200,	/* d */
22,		/* e */
16+0200,	/* f */
22+0100,	/* g */
26+0200,	/* h */
14+0200,	/* i */
14+0300,	/* j */
24+0200,	/* k */
14+0200,	/* l */
38,		/* m */
26,		/* n */
22,		/* o */
24+0100,	/* p */
22+0100,	/* q */
19,		/* r */
19,		/* s */
17+0200,	/* t */
26,		/* u */
20,		/* v */
27,		/* w */
22,		/* x */
22+0100,	/* y */
20,		/* z */
16,		/* { */
9,		/* | */
16,		/* } */
27,		/* ~ */
6,		/* narrow space */
14,		/* hyphen */
9,		/* bullet */
9,		/* square */
28,		/* 3/4 em dash */
28,		/* rule */
0,		/* 1/4 */
0,		/* 1/2 */
0,		/* 3/4 */
14,		/* minus */
24,		/* fi */
24,		/* fl */
26,		/* ff */
37,		/* ffi */
37,		/* ffl */
16,		/* degree */
18,		/* dagger */
0,		/* section (unimplem) */
10,		/* foot mark */
0,		/* acute acc (unimplem) */
0,		/* grave acc (unimplem) */
0,		/* underrule (unimplem) */
0,		/* slash (unimplem) */
3,		/* half narrow space */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* registered */
0,		/* copyright */
0,		/* null */
0,		/* cent */
};

/*  Special font  */
char W4[256-32] {
0,		/* null */
0,		/* null */
18,		/* " */
23,		/* # */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
27,		/* < */
0,		/* null */
27,		/* > */
0,		/* null */
30,		/* @ */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
23,		/* \ */
0,		/* null */
25,		/* ^ */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
16,		/* { */
0,		/* null */
16,		/* } */
27,		/* ~ */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
0,		/* null */
18,		/* section */
0,		/* null */
13,		/* acute accent */
13,		/* grave accent */
29,		/* underrule */
31,		/* slash (longer) */
0,		/* null */
0,		/* null */
26,		/* alpha */
23+0300,	/* beta */
22+0100,	/* gamma */
22+0200,	/* delta */
21,		/* epsilon */
21+0300,	/* zeta */
25+0100,	/* eta */
26+0200,	/* theta */
13,		/* iota */
22,		/* kappa */
22+0200,	/* lambda */
26+0100,	/* mu */
22,		/* nu */
19+0300,	/* xi */
21,		/* omicron */
25,		/* pi */
22+0100,	/* rho */
23,		/* sigma */
22,		/* tau */
22,		/* upsilon */
25+0100,	/* phi */
21+0100,	/* chi */
26+0300,	/* psi */
26,		/* omega */
22+0200,	/* Gamma */
23+0200,	/* Delta */
26+0200,	/* Theta */
23+0200,	/* Lambda */
26+0300,	/* Xi */
29+0200,	/* Pi */
25+0200,	/* Sigma */
0,		/* null */
22+0200,	/* Upsilon */
25+0200,	/* Phi */
27+0200,	/* Psi */
26+0200,	/* Omega */
36,		/* square root */
21+0100,	/* terminal sigma */
28,		/* root en */
27,		/* >= */
27,		/* <= */
30,		/* identically equal */
29,		/* minus */
27,		/* approx = */
27,		/* approximates */
30,		/* not equal */
30,		/* right arrow */
30,		/* left arrow */
18,		/* up arrow */
18,		/* down arrow */
29,		/* equal */
25,		/* multiply */
30,		/* divide */
27,		/* plus-minus */
27,		/* cup (union) */
27,		/* cap (intersection) */
27,		/* subset of */
27,		/* superset of */
27,		/* improper subset */
27,		/* improper superset */
29,		/* infinity */
22,		/* partial derivative */
22+0200,	/* gradient */
30,		/* not */
27,		/* integral sign */
29,		/* proportional to */
25,		/* empty set */
27,		/* angstrom */
29,		/* plus */
0,		/* null */
0,		/* null */
0,		/* box vert rule (forced 0, was 2.) */
0,		/* null */
18,		/* dbl dagger */
0,		/* right hand */
27,		/* left hand */
19,		/* math *  */
25,		/* bell system sign */
9,		/* or */
25,		/* circle */
5,		/* left top (of big curly) */
5,		/* left bottom */
9,		/* right top */
9,		/* right bot */
5,		/* left center of big curly bracket */
9,		/* right center of big curly bracket */
5,		/* bold vertical */
5,		/* left floor (left bot of big sq bract) */
8,		/* right floor (rb of ) */
5,		/* left ceiling (lt of ) */
8,		/* right ceiling (rt of ) */
};
