/*
 *		Assembler tables
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

/*
 * For double precision floating-point instructions, define OPDBL
 * For 8/32 instructions, define OP832
 */

#define EXTERN	extern
#include "as.h"

/*
 * Opcode classes
 */

#define RI2	0x1
#define RX	0x2
#define BRX	0x3
#define RI1	0x4
#define RR	0x8
#define SF	0x8
#define BRR	0xc
#define BSF	0xd

#define NOP	0xf		/* NOP or NOPR instruction */

/*
 * Branch table for opcode routines
 *	- indexed by class of opcode
 */
int (*oproutine[])() {
	&donull,			/* unused opcode - ignore */
	&dori2,			/* RI2 */
	&dorx,			/* RX1, RX2, RX3 */
	&dobrx,			/* RX-type branch */
	&dori1,			/* RI1 */
	&dodata,		/* PSEUDO: data definition */
	&doloc,			/* PSEUDO: location counter */
	&dopseud,		/* PSEUDO: other */
	&dorr,			/* RR */
	&docond,		/* PSEUDO: conditional assembly */
	&dobad,			/*	-	*/
	&dobad,			/*	-	*/
	&dobrr,			/* RR-type branch */
	&dobsf,			/* SF-type branch */
	&dobad,			/*	-	*/
	&donop			/* STUPID: zero-or-one-operand (=NOP,NOPR) */
};

/*
 * Opcodes and pseudo-ops
 */
struct optab opcodes[] {

/* data-definition pseudo-ops */
0,	"db",		0x05,
0,	"dc",		0x15,
0,	"dac",		0x25,
0,	"dcf",		0x35,
0,	"dcx",		0x45,
0,	"dcy",		0x55,
0,	"dcz",		0x65,

/* location counter pseudo-ops */
0,	"impur",	0x06,
0,	"pure",		0x16,
0,	"align",	0x26,
0,	"ds",		0x36,
0,	"das",		0x46,
0,	"dsf",		0x46,
0,	"dsh",		0xc6,
0,	"org",		0x56,
0,	"comn",		0x66,
0,	"struc",	0x76,
0,	"ends",		0x86,
0,	"bss",		0x96,
0,	"bdata",	0xa6,
0,	"borg",		0xb6,
0,	"dlist",	0xd6,
0,	"cnop",		0xe6,

/* Other pseudo-ops */
0,	"equ",		0x07,
0,	"extrn",	0x17,
0,	"entry",	0x27,
0,	"do",		0x37,
0,	"squez",	0x47,
0,	"nosqz",	0x57,
0,	"norx3",	0x67,

/* Conditional assembly pseudo-ops */
0,	"ifz",		0x09,
0,	"ifnz",		0x19,
0,	"ifp",		0x29,
0,	"ifnp",		0x39,
0,	"ifm",		0x49,
0,	"ifnm",		0x59,
0,	"ife",		0x69,
0,	"ifo",		0x79,
0,	"ifu",		0x89,
0,	"ifd",		0x99,

0,	"else",		0xa9,
0,	"endc",		0xb9,
0,	"if",		0xc9,
0,	"end",		0xd9,

/* 7/32 Machine Instructions */
0,	"a",		0x5a00 | RX,
0,	"abl",		0x6500 | RX,
0,	"ae",		0x6a00 | RX,
0,	"aer",		0x2a00 | RR,
0,	"ah",		0x4a00 | RX,
0,	"ahi",		0xca00 | RI1,
0,	"ahm",		0x6100 | RX,
0,	"ai",		0xfa00 | RI2,
0,	"ais",		0x2600 | SF,
0,	"al",		0xd500 | RX,
0,	"am",		0x5100 | RX,
0,	"ar",		0x0a00 | RR,
0,	"atl",		0x6400 | RX,
0,	"bal",		0x4100 | RX,
0,	"balr",		0x0100 | RR,
0,	"bfbs",		0x2200 | SF,
0,	"bfc",		0x4300 | RX,
0,	"bfcr",		0x0300 | RR,
0,	"bffs",		0x2300 | SF,
0,	"btbs",		0x2000 | SF,
0,	"btc",		0x4200 | RX,
0,	"btcr",		0x0200 | RR,
0,	"btfs",		0x2100 | SF,
0,	"bxh",		0xc000 | RX,
0,	"bxle",		0xc100 | RX,
0,	"c",		0x5900 | RX,
0,	"cbt",		0x7700 | RX,
0,	"ce",		0x6900 | RX,
0,	"cer",		0x2900 | RR,
0,	"ch",		0x4900 | RX,
0,	"chi",		0xc900 | RI1,
0,	"chvr",		0x1200 | RR,
0,	"ci",		0xf900 | RI2,
0,	"cl",		0x5500 | RX,
0,	"clb",		0xd400 | RX,
0,	"clh",		0x4500 | RX,
0,	"clhi",		0xc500 | RI1,
0,	"cli",		0xf500 | RI2,
0,	"clr",		0x0500 | RR,
0,	"cr",		0x0900 | RR,
0,	"crc12",	0x5e00 | RX,
0,	"crc16",	0x5f00 | RX,
0,	"d",		0x5d00 | RX,
0,	"de",		0x6d00 | RX,
0,	"der",		0x2d00 | RR,
0,	"dh",		0x4d00 | RX,
0,	"dhr",		0x0d00 | RR,
0,	"dr",		0x1d00 | RR,
0,	"epsr",		0x9500 | RR,
0,	"exbr",		0x9400 | RR,
0,	"exhr",		0x3400 | RR,
0,	"flr",		0x2f00 | RR,
0,	"fxr",		0x2e00 | RR,
0,	"l",		0x5800 | RX,
0,	"la",		0xe600 | RX,
0,	"lb",		0xd300 | RX,
0,	"lbr",		0x9300 | RR,
0,	"lcs",		0x2500 | SF,
0,	"le",		0x6800 | RX,
0,	"ler",		0x2800 | RR,
0,	"lh",		0x4800 | RX,
0,	"lhi",		0xc800 | RI1,
0,	"lhl",		0x7300 | RX,
0,	"li",		0xf800 | RI2,
0,	"lis",		0x2400 | SF,
0,	"lm",		0xd100 | RX,
0,	"lme",		0x7200 | RX,
0,	"lpsw",		0xc200 | RX,
0,	"lpswr",	0x1800 | RR,
0,	"lra",		0x6300 | RX,
0,	"lr",		0x0800 | RR,
0,	"m",		0x5c00 | RX,
0,	"me",		0x6c00 | RX,
0,	"mer",		0x2c00 | RR,
0,	"mh",		0x4c00 | RX,
0,	"mhr",		0x0c00 | RR,
0,	"mr",		0x1c00 | RR,
0,	"n",		0x5400 | RX,
0,	"nh",		0x4400 | RX,
0,	"nhi",		0xc400 | RI1,
0,	"ni",		0xf400 | RI2,
0,	"nr",		0x0400 | RR,
0,	"o",		0x5600 | RX,
0,	"oc",		0xde00 | RX,
0,	"ocr",		0x9e00 | RR,
0,	"oh",		0x4600 | RX,
0,	"ohi",		0xc600 | RI1,
0,	"oi",		0xf600 | RI2,
0,	"or",		0x0600 | RR,
0,	"rb",		0xd700 | RX,
0,	"rbl",		0x6700 | RX,
0,	"rbr",		0x9700 | RR,
0,	"rbt",		0x7600 | RX,
0,	"rd",		0xdb00 | RX,
0,	"rdr",		0x9b00 | RR,
0,	"rh",		0xd900 | RX,
0,	"rhr",		0x9900 | RR,
0,	"rll",		0xeb00 | RI1,
0,	"rrl",		0xea00 | RI1,
0,	"rtl",		0x6600 | RX,
0,	"s",		0x5b00 | RX,
0,	"sbt",		0x7500 | RX,
0,	"scp",		0xe300 | RX,
0,	"se",		0x6b00 | RX,
0,	"ser",		0x2b00 | RR,
0,	"sh",		0x4b00 | RX,
0,	"shi",		0xcb00 | RI1,
0,	"si",		0xfb00 | RI2,
0,	"sint",		0xe200 | RI1,
0,	"sis",		0x2700 | SF,
0,	"sla",		0xef00 | RI1,
0,	"slha",		0xcf00 | RI1,
0,	"slhl",		0xcd00 | RI1,
0,	"sll",		0xed00 | RI1,
0,	"slhls",	0x9100 | SF,
0,	"slls",		0x1100 | SF,
0,	"sr",		0x0b00 | RR,
0,	"sra",		0xee00 | RI1,
0,	"srha",		0xce00 | RI1,
0,	"srhl",		0xcc00 | RI1,
0,	"srl",		0xec00 | RI1,
0,	"srhls",	0x9000 | SF,
0,	"srls",		0x1000 | SF,
0,	"ss",		0xdd00 | RX,
0,	"ssr",		0x9d00 | RR,
0,	"st",		0x5000 | RX,
0,	"stb",		0xd200 | RX,
0,	"stbr",		0x9200 | RR,
0,	"ste",		0x6000 | RX,
0,	"sth",		0x4000 | RX,
0,	"stm",		0xd000 | RX,
0,	"stme",		0x7100 | RX,
0,	"svc",		0xe100 | RX,
0,	"tbt",		0x7400 | RX,
0,	"thi",		0xc300 | RI1,
0,	"ti",		0xf300 | RI2,
0,	"tlate",	0xe700 | RX,
0,	"ts",		0xe000 | RX,
0,	"wb",		0xd600 | RX,
0,	"wbr",		0x9600 | RR,
0,	"wd",		0xda00 | RX,
0,	"wdr",		0x9a00 | RR,
0,	"wh",		0xd800 | RX,
0,	"whr",		0x9800 | RR,
0,	"x",		0x5700 | RX,
0,	"xh",		0x4700 | RX,
0,	"xhi",		0xc700 | RI1,
0,	"xi",		0xf700 | RI2,
0,	"xr",		0x0700 | RR,

/* Extended Branch Mnemonics */
0,	"bc",		0x4280 | BRX,
0,	"bcr",		0x0280 | BRR,
0,	"bcs",		0x2080 | BSF,
0,	"bnc",		0x4380 | BRX,
0,	"bncr",		0x0380 | BRR,
0,	"bncs",		0x2280 | BSF,
0,	"be",		0x4330 | BRX,
0,	"ber",		0x0330 | BRR,
0,	"bes",		0x2230 | BSF,
0,	"bne",		0x4230 | BRX,
0,	"bner",		0x0230 | BRR,
0,	"bnes",		0x2030 | BSF,
0,	"bl",		0x4280 | BRX,
0,	"blr",		0x0280 | BRR,
0,	"bls",		0x2080 | BSF,
0,	"bnl",		0x4380 | BRX,
0,	"bnlr",		0x0380 | BRR,
0,	"bnls",		0x2280 | BSF,
0,	"bm",		0x4210 | BRX,
0,	"bmr",		0x0210 | BRR,
0,	"bms",		0x2010 | BSF,
0,	"bnm",		0x4310 | BRX,
0,	"bnmr",		0x0310 | BRR,
0,	"bnms",		0x2210 | BSF,
0,	"bp",		0x4220 | BRX,
0,	"bpr",		0x0220 | BRR,
0,	"bps",		0x2020 | BSF,
0,	"bnp",		0x4320 | BRX,
0,	"bnpr",		0x0320 | BRR,
0,	"bnps",		0x2220 | BSF,
0,	"bo",		0x4240 | BRX,
0,	"bor",		0x0240 | BRR,
0,	"bos",		0x2040 | BSF,
0,	"bno",		0x4340 | BRX,
0,	"bnor",		0x0340 | BRR,
0,	"bnos",		0x2240 | BSF,
0,	"bz",		0x4330 | BRX,
0,	"bzr",		0x0330 | BRR,
0,	"bzs",		0x2230 | BSF,
0,	"bnz",		0x4230 | BRX,
0,	"bnzr",		0x0230 | BRR,
0,	"bnzs",		0x2030 | BSF,
0,	"b",		0x4300 | BRX,
0,	"br",		0x0300 | BRR,
0,	"bs",		0x2200 | BSF,
0,	"nop",		0x4200 | NOP,
0,	"nopr",		0x0200 | NOP,

/* 32-bit Common Mode Instructions */
0,	"aa",		0x5a00 | RX,
0,	"aai",		0xfa00 | RI2,
0,	"aar",		0x0a00 | RR,
0,	"aam",		0x5100 | RX,
0,	"ca",		0x5900 | RX,
0,	"cai",		0xf900 | RI2,
0,	"car",		0x0900 | RR,
0,	"cla",		0x5500 | RX,
0,	"clai",		0xf500 | RI2,
0,	"clar",		0x0900 | RR,
0,	"lda",		0x5800 | RX,
0,	"ldai",		0xe600 | RX,
0,	"ldar",		0x0800 | RR,
0,	"na",		0x5400 | RX,
0,	"nai",		0xf400 | RI2,
0,	"nar",		0x0400 | RR,
0,	"oa",		0x5600 | RX,
0,	"oai",		0xf600 | RI2,
0,	"oar",		0x0600 | RR,
0,	"sa",		0x5b00 | RX,
0,	"sai",		0xfb00 | RI2,
0,	"sar",		0x0b00 | RR,
0,	"slaa",		0xef00 | RI1,
0,	"slal",		0xed00 | RI1,
0,	"sraa",		0xee00 | RI1,
0,	"sral",		0xec00 | RI1,
0,	"sta",		0x5000 | RX,
0,	"tai",		0xf300 | RI2,
0,	"xa",		0x5700 | RX,
0,	"xai",		0xf700 | RI2,
0,	"xar",		0x0700 | RR,
0,	"ma",		0x5c00 | RX,
0,	"mar",		0x1c00 | RR,
0,	"da",		0x5d00 | RX,
0,	"dar",		0x1d00 | RR,

/* Double-precision floating point instructions */
#ifdef	OPDBL
0,	"ad",		0x7a00 | RX,
0,	"adr",		0x3a00 | RR,
0,	"cd",		0x7900 | RX,
0,	"cdr",		0x3900 | RR,
0,	"dd",		0x7d00 | RX,
0,	"ddr",		0x3d00 | RR,
0,	"fldr",		0x3f00 | RR,
0,	"fxdr",		0x3e00 | RR,
0,	"ld",		0x7800 | RX,
0,	"ldr",		0x3800 | RR,
0,	"lmd",		0x7f00 | RX,
0,	"md",		0x7c00 | RX,
0,	"mdr",		0x3c00 | RR,
0,	"sd",		0x7b00 | RX,
0,	"sdr",		0x3b00 | RR,
0,	"std",		0x7000 | RX,
0,	"stmd",		0x7e00 | RX,
#endif

/* 8/32 Machine Instructions */
#ifdef OP832
0,	"bdcs",		0xe500 | RX,
0,	"ecs",		0xe900 | RI1,
0,	"mpbsr",	0x3000 | RR,
0,	"pb",		0x6200 | RX,
0,	"pbr",		0x3200 | RR,
0,	"rdcs",		0xe820 | BRR,
0,	"wdcs",		0xe800 | BRR,
#endif

/* Ignored pseudo-ops */
0,	"title",	0,
0,	"eject",	0,
0,	"space",	0,
0,	"nlist",	0,
0,	"list",		0,
0,	"lstc",		0,
0,	"nlstc",	0,
0,	"prog",		0,
0,	"lcnt",		0,
0,	"width",	0,
0,	"scrat",	0,
0,	"ersqz",	0,

0,	0
};

/*
 * Predefined symbols
 */
struct symbol predefs[] {
0,	"adc",	1,	4,
0,	"ladc",	1,	2,
0,	0
};
