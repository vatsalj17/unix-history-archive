#
/*

	Dynamic Debugging Tool
		Interdata version:
		Richard Miller
		september 1977

*/




extern int dot,sdot,dotinc,errflg;
extern status;
extern char *lp;
extern int regbuf[];
extern int coroffset,fcore,wcore,brktx;
#define BRKLEN 20
#define CONDLEN 82
extern struct {
	int value;		/* old value of breakpoint */
	int addr;		/* address of breakpoint */
	char cond[CONDLEN];	/* breakpoint condition */
} brktab[BRKLEN];
extern int callev, entpt[];

#define	BREAK	3
extern char **uregs;

#define PC 7

#define SSR0	0
#define SSR1	1
#define SSR2	2
#define SSR3	3
#define SSR4	4
#define SSR5	5
#define SSR6	6
#define SSR7	7
#define	SSR8	8
#define SSR9	9
#define SSR10	10
#define SSR11	11
#define SSR12	12
#define SSR13	13
#define SSR14	14
#define SSR15	15
#define SSSP	7
#define SSPC	16
#define SSPS	17
#define SS8	18
#define SS9	19
#define SSL	20
#define SSM	21
#define SSQ	22
#define SSA	23
#define SSD	24
#define SSRG	25
#define SSF	26
#define SSSL	27
#define NUMSS	28

int ssval[NUMSS];		/* special symbol values */

char *ssname[]			/* special symbol names */
{
	"r0",		/* 0 user reg 0 */
	"r1",		/* 1 */
	"r2",		/* 2 */
	"r3",		/* 3 */
	"r4",		/* 4 */
	"r5",		/* 5 */
	"r6",		/* 6 */
	"sp",		/* 6 */
	"r8",
	"r9",
	"ra",
	"rb",
	"rc",
	"rd",
	"re",
	"rf",
	"pc",		/* 7 */
	"ps",		/* 8 users processor status */
	"8",		/* 9 search lower limit */
	"9",		/* 10 search upper limit */
	"l",		/* 11 low address for symbols */
	"m",		/* 12 search mask value */
	"q",		/* 13 last value typed to user */
	"a",			/* 14 have assembler symbols (no _) if set */
	"d",			/* 15 D space symbol offset */
	"r",				/* 16 symbol range value */
	"f",				/* current function for local symbols */
	"sl"				/* lowest address on stack */
};

char *termstr[] {	/* termination status strings */
	"normal exit",	/* 0 */
	"hangup",		/* 1 */
	"interrupt",	/* 2 */
	"quit",			/* 3 */
	"illegal instruction",	/* 4 */
	"trace trap",	/* 5 */
	"IOT instruction",	/* 6 */
	"EMT instruction",	/* 7 */
	"arithmetic exception",	/* 8 */
	"killed",		/* 9 */
	"addess fault",	/* 10 */
	"segmentation violation",	/* 11 */
	"bad argument to system call"	/* 12 */
};

int pbkptc, pbkptv;	/* location  value of temporarily cleared bpt */
int tempbra[2],tempbrv[2];

/*
 * instruction formats
 */
#define	SF	1
#define	RR	2
#define	RX1	3
#define	RX2	4
#define	RX3	5
#define	RI1	6
#define	RI2	7

#define	BT	010	/* branch instruction (true)	*/
#define	BF	020	/* branch instruction (false)	*/
#define	BBACK	040	/* branch backwards short	*/

/*
 * structure for information about disassembled instruction
 */
struct {
	int	type;		/* instruction format	*/
	char	op;		/* opcode	*/
	char	reg[3];		/* registers	*/
	char	*disp;		/* displacement */
} instr;

/*
 * Interdata opcode mnemonics & types
 */
struct {
	char *opmnem;
	int  optype;
} optab[] {
	 0,	0,
	"balr",	RR,
	"btcr",	BT+RR,
	"bfcr",	BF+RR,
	"nr",	RR,
	"clr",	RR,
	"or",	RR,
	"xr",	RR,
	"lr",	RR,
	"cr",	RR,
	"ar",	RR,
	"sr",	RR,
	"mhr",	RR,
	"dhr",	RR,
	 0,	0,
	 0,	0,
	"srls",	SF,
	"slls",	SF,
	"chvr",	RR,
	"bfcr",	BF+RR,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	"lpsr",	RR,
	 0,	0,
	 0,	0,
	 0,	0,
	"mr",	RR,
	"dr",	RR,
	 0,	0,
	 0,	0,
	"btbs",	BT+BBACK+SF,
	"btfs",	BT+SF,
	"bfbs",	BF+BBACK+SF,
	"bffs",	BF+SF,
	"lis",	SF,
	"lcs",	SF,
	"ais",	SF,
	"sis",	SF,
	"ler",	RR,
	"cer",	RR,
	"aer",	RR,
	"ser",	RR,
	"mer",	RR,
	"der",	RR,
	"fxr",	RR,
	"flr",	RR,
	"btbs",	BT+BBACK+SF,
	"btfs",	BT+SF,
	 0,	0,
	 0,	0,
	"exhr",	RR,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	"sth",	RX1,
	"bal",	RX1,
	"btc",	BT+RX1,
	"bfc",	BF+RX1,
	"nh",	RX1,
	"clh",	RX1,
	"oh",	RX1,
	"xh",	RX1,
	"lh",	RX1,
	"ch",	RX1,
	"ah",	RX1,
	"sh",	RX1,
	"mh",	RX1,
	"dh",	RX1,
	 0,	0,
	 0,	0,
	"st",	RX1,
	"am",	RX1,
	"btc",	BT+RX1,
	"bfc",	BF+RX1,
	"n",	RX1,
	"cl",	RX1,
	"o",	RX1,
	"x",	RX1,
	"l",	RX1,
	"c",	RX1,
	"a",	RX1,
	"s",	RX1,
	"m",	RX1,
	"d",	RX1,
	"cr12",	RX1,
	"cr16",	RX1,
	"ste",	RX1,
	"ahm",	RX1,
	 0,	0,
	 0,	0,
	"atl",	RX1,
	"abl",	RX1,
	"rtl",	RX1,
	"rbl",	RX1,
	"le",	RX1,
	"ce",	RX1,
	"ae",	RX1,
	"se",	RX1,
	"me",	RX1,
	"de",	RX1,
	 0,	0,
	 0,	0,
	 0,	0,
	"stme",	RX1,
	"lme",	RX1,
	"lhl",	RX1,
	"tbt",	RX1,
	"sbt",	RX1,
	"rbt",	RX1,
	"cbt",	RX1,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	"srls",	SF,
	"slls",	SF,
	"stbr",	RR,
	"lbr",	RR,
	"exbr",	RR,
	"epsr",	RR,
	"wbr",	RR,
	"rbr",	RR,
	"whr",	RR,
	"rhr",	RR,
	"wdr",	RR,
	"rdr",	RR,
	 0,	0,
	"ssr",	RR,
	"ocr",	RR,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0,
	"bxh",	RX1,
	"bxle",	RX1,
	"lpsw",	RX1,
	"thi",	RI1,
	"nhi",	RI1,
	"clhi",	RI1,
	"ohi",	RI1,
	"xhi",	RI1,
	"lhi",	RI1,
	"chi",	RI1,
	"ahi",	RI1,
	"shi",	RI1,
	"srhl",	RI1,
	"slhl",	RI1,
	"srha",	RI1,
	"slha",	RI1,
	"stm",	RX1,
	"lm",	RX1,
	"stb",	RX1,
	"lb",	RX1,
	"clb",	RX1,
	"al",	RX1,
	"wb",	RX1,
	"rb",	RX1,
	"wh",	RX1,
	"rh",	RX1,
	"wd",	RX1,
	"rd",	RX1,
	 0,	0,
	"ss",	RX1,
	"oc",	RX1,
	 0,	0,
	"ts",	RX1,
	"svc",	RX1,
	"sint",	RX1,
	"scp",	RX1,
	 0,	0,
	 0,	0,
	"la",	RX1,
	"tlat",	RX1,
	 0,	0,
	 0,	0,
	"rrl",	RI1,
	"rll",	RI1,
	"srl",	RI1,
	"sll",	RI1,
	"sra",	RI1,
	"sla",	RI1,
	 0,	0,
	 0,	0,
	 0,	0,
	"ti",	RI2,
	"ni",	RI2,
	"cli",	RI2,
	"oi",	RI2,
	"xi",	RI2,
	"li",	RI2,
	"ci",	RI2,
	"ai",	RI2,
	"si",	RI2,
	 0,	0,
	 0,	0,
	 0,	0,
	 0,	0
};

/*
 * Branch condition names
 */
struct {
	char *true;
	char *false;
} brtab[] {
	"nop",	"b",
	"bm",	"bnm",
	"bp",	"bnp",
	"bne",	"be",
	"bo",	"bno",
	0,	0,
	0,	0,
	0,	0,
	"bl",	"bnl",
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0
};

/*
 * Disassemble instruction at loc, returning length of instruction
 */
disasm(loc)
{
	register ins;

	ins = get(loc);

	instr.op = ins>>24;
	instr.reg[0] = (ins>>20)&017;
	instr.reg[1] = (ins>>16)&017;
	instr.type = optab[instr.op].optype;
	switch (instr.type & 07) {

	case SF:
	case RR:
		return(2);

	case RI1:
		instr.disp = ins&0177777;
		return(4);

	case RI2:
		instr.disp = get(loc+2);
		return(6);

	case RX1:
		ins = get(loc+2);
		switch(ins>>30) {

		/* RX2 -- negative offset */
		case 3:
			instr.type =+ RX2-RX1;
			instr.disp = ((-1)<<16) | ins>>16;
			instr.disp =+ loc + 4;
			return(4);
	
		/* RX2 -- positive offset */
		case 2: 
			instr.type =+ RX2-RX1;
			instr.disp = (ins>>16) & 077777;
			instr.disp =+ loc + 4;
			return(4);
	
		/* RX1 */
		case 0:
			instr.disp = ins>>16;
			return(4);
	
		/* RX3 */
		case 1:
			instr.type =+ RX3-RX1;
			instr.reg[2] = (ins>>24) & 017;
			ins =& 077777777;
			if (ins & 01000000)	/* negative offset */
				ins =| (0377<<24);
			instr.disp = ins;
			return(6);
		}

	default:
		return(0);
	}
}

/*  print an instruction in assembler format */

pinstr()
{
	register n;
	register type;

	if ((dotinc = disasm(dot)) == 0) {	/* illegal opcode */
		printf(" ??\t");
		dotinc = 2;
		sdot = dot + 2;
		return;
	}

	sdot = dot + dotinc;
	type = instr.type;
	n = instr.reg[0];
	if ((type & (BT|BF)) == 0)			/* non-branch */
		printf("%s\t%s,", optab[instr.op].opmnem, ssname[instr.reg[0]]);
	else if (type&BT && (n = brtab[n].true) == 0 ||
	    type&BF && (n = brtab[n].false) == 0) 		/* branch */
		printf("%s\t%o,", optab[instr.op].opmnem, instr.reg[0]);
	else {					/* extended branch */
		printf(n);
		switch (type & 07) {

		case RR:
			printf("r\t");
			break;

		case RX1:
		case RX2:
		case RX3:
			printf("\t");
			break;

		case SF:
			printf("s\t");
			n = instr.reg[1]<<1;
			instr.reg[1] = 0;
			if (type & BBACK)
				instr.disp = dot - n;
			else
				instr.disp = dot + n;
			type = RI1;
			break;
		}
	}

	switch (type & 07) {

	case RR:
		printf("%s", ssname[instr.reg[1]]);
		break;

	case SF:
		printf("%o", instr.reg[1]);
		break;

	case RI1:
	case RI2:
	case RX1:
	case RX2:
		psym(instr.disp);
		if (n = instr.reg[1])
			printf("(%s)", ssname[n]);
		break;

	case RX3:
		psym(instr.disp);
		if (n = instr.reg[1]) {
			printf("(%s", ssname[n]);
			if (n = instr.reg[2])
				printf(",%s", ssname[n]);
			putchar(')');
		}
		break;

	}
	putchar('\t');
}

/* get the value of a special symbol  */

getspsym()
{

	return(ssval[spsymidx()]);
}

/* store a value into a special symbol */

putspsym(value)
{
	register t1,t2;

	t1 = spsymidx();
	if(t1==SSF) {	/* current function */
		for(t2=0; t2<callev; t2++)	/* make sure its active */
			if(entpt[t2] == value)
				goto putit;
		printf("*** not an active function\n");
		return;
	}
putit:
	ssval[t1] = value;
	return;
}

/* return the index in ssval of a special symbol. */
/*   the special symbol name is pointed to by lp */

spsymidx()
{

	register int i;
	register char *p,*tlp;

	for(i=0;i<NUMSS;i++){
		tlp = lp;	/* points to special symbol name */
		p = ssname[i];	/* list of correct names */
		while(*p){
			if(*p++ != *tlp++)	/* not this one */
				goto nextss;
		}
		lp = tlp;	/* point past name */
		return(i);	/* found it */
nextss:
		continue;
	}
	printf("** no such special symbol name\n");
	return(0);
}

/* initialize the default special symbol values */

initssv()
{

	ssval[SSL] = 0x100;	/* low symbolic address */
	ssval[SS9] = 0x8000;	/*** high search address ***/
	ssval[SSM] = -1;	/* search mask */
	ssval[SSRG] = 0x1000;	/* symbol range */
	ssval[SSSL] = 0xe0000;	/* lowest address on stack */
	ssval[SSA] = 1;	/* no '_' prefix for symbols */
}

/*  set a temporary breakpoint */
/*   flag == 0  => called from ;s */
/*   flag == 1  => called from ;p */
stempbpt(flag)
{

	register int tpc,i,j;

/*  if pc points to a permanent breakpoint, clear it temporarily */

	pbkptc = 0;
	tpc = ssval[SSPC];
	for(i=0; i<=brktx; i++)
		if(brktab[i].addr == tpc){
			pbkptc = tpc;		/* remember who we cleared */
			pbkptv = brktab[i].value;
			dot = tpc&~03;
			cfput(pbkptv);
		}
	if(pbkptc==0 && flag)	/* ;p really desnt need a temp bkpt */
		return(0);
/***	ssval[SSPS] =| 020;		/* turn on T bit ***/
	return(1);
}


/* remove a temporary breakpoint */

rtempbpt()
{

	register int i;

	if(pbkptc) {		/* must reset a permanent one */
		dot = pbkptc&~03;
		if (dot == pbkptc)
			i =
			(BREAK
				<<16)
				|
				(pbkptv
				&
				0xffff)
				;
		else
			i = (pbkptv&0xffff0000) | BREAK;
		cfput(i);
		pbkptc = 0;
	}
}

/* print termination status */
prtermst()
{

	register int i;

	i = status&077;		/* status code */
	if(i>12) {
		printf("Unknown termination status\n");
		return;
	}
	printf("Termination status: %o  %s\n",status,termstr[i]);
}
