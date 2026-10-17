/*
 *		Assembler mainline
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

#define EXTERN	
#include "as.h"

char	*ofname	= "a.out";	/* default object filename */
char	**ifiles;		/* list of input filenames */
int	cfile;			/* index of current input filename */
int	nifiles;		/* number of input files */

int	iopt	= 1;		/* default: full SQUEZ optimization */

extern int (*oproutine[])();	/* branch table to opcode routines */

char	usage[] =
	"Usage: ras [-u] [-m] [-sN] [-o object-file] input-file [...]\n";

/*
 * Main program - initialize; call assembly passes; terminate
 *	- pass 0: build symbol table
 *	- pass 1: 'optimize' by building a bitmap of instructions which can
 *		be squeezed to shorter forms
 *	- pass 2: generate a.out file
 * All three passes re-read and re-parse the source code.
 * Any assembly error prevents further passes.
 */
main(argc, argv)
char **argv;
{
	register c;
	extern fout;
	int symout();

	/*
	 * Get flags & files
	 */
	while (--argc > 0 && **++argv == '-') {
		switch (argv[0][1]) {
		case 'u':
			ucase++;
			break;
		case 'm':
			mflag++;
			break;
		case 'l':
			lflag++;
			break;
		case 'o':
			if (--argc == 0 || **++argv == '-') {
				printf("No object filename\n");
				exit(1);
			}
			ofname = *argv;
			break;
		case 's':
			if ((c = argv[0][2]) == '0')
				iopt = 0;		/* NOSQUEZ */
			else if (c == '1')
				iopt = -1;		/* NORX3 */
			break;
		}
	}
	if ((nifiles = argc) < 1) {
		printf(usage);
		exit(1);
	}
	ifiles = argv;
	/*
	 * Enter opcodes into symbol table
	 */
	syminit();
	/*
	 * Symbol definition pass
	 */
	assemble();
	if (errcnt)
		exit(errcnt);
	/*
	 * Squeeze pass
	 */
	pass++;
	sqzalloc();
	assemble();
	if (errcnt)
		exit(errcnt);
	/*
	 * Code generation pass(es)
	 *	- more than one pass may be required in pathological cases
	 *	  where squeezed instructions must be unsqueezed.
	 *	  Since each pass can only make instructions longer, the
	 *	  process is guaranteed to terminate eventually.
	 */
	do {
		if ((ofile = creat(ofname, 0666)) < 0) {
			printf("%s: can't create\n", ofname);
			exit(1);
		}
		pass++;
		passg = 1;
		outhdr();
		assemble();
	} while (!passg);
	oflush(&impure);
	oflush(&pure);
	/*
	 * listing pass
	 */
	if (lflag) {
		passg = 0;
		passl = 1;
		fout = dup(1);
		if ((signal(2, 1) & 01) == 0)
			signal(2, &symout);
		assemble();
		flush();
	}

	/*
	 * append symbol table to object file
	 */
	symout();

	exit(errcnt);
}

/*
 * Assembler pass
 *	- skip comments & null lines
 *	- parse label & opcode
 *	- call opcode routine to parse operands and generate code
 */
assemble()
{
	register t, op;
	register optype;

	/*
	 * Initialize for assembly pass
	 */
	firstfile();
	sqzinit();
	seginit();
	noasm = endswitch = 0;
	opt = iopt;

	setexit();
	for (; getline(); putline()) {
		if (endswitch) {
			error(erre);
			break;
		}
		curlab = dcflag = 0;
		/*
		 * comment
		 */
		if ((t = token()) == STAR || t == EOL)
			continue;
		/*
		 * save statement label symbol to be defined by opcode routine
		 */
		if (t == SYMBOL) {
			curlab = cursym;
			t = token();
		}
		if (t != SPACE)
			xerror(errx);
		/*
		 * a line with nothing but white space is a comment
		 */
		if (!curlab && eol())
			continue;
		/*
		 * get opcode
		 */
		if (!getsym())
			xerror(errc);
		if (symlook(0)) {		/* built-in opcode */
			optype = SABS;
			op = ((struct optab *)cursym)->opval;
		} else if (symlook(1)) {	/* user-defined opcode */
			optype = cursym->type;
			op = cursym->value;
		} else {			/* undefined opcode */
			optype = SUNDEF;
			op = 0;
		}
		/*
		 * If conditional assembly off, ignore all but conditional
		 *	assembly pseudo-ops
		 */
		if ((noasm|ifcount) && (op&0xf)!=9)
			continue;
		if (optype != SABS)
			xerror(errc);
		if ((t = token()) != SPACE && t != EOL)
			xerror(errx);
		/*
		 * call opcode routine to parse the operands
		 */
		(*oproutine[op&0xf])(op>>4);
		if ((t = token()) != SPACE && t != EOL)
			xerror(errx);
	}

	/*
	 * End of pass - check for missing ENDC or ENDS
	 */
	if (ifnest || curcomm)
		error(erri);
	return(errcnt);
}

/*
 * Open first input file
 */
firstfile()
{
	line = 0;
	if (pass == 0 || nifiles > 1) {
		cfile = -1;
		nextfile();
	} else
		cfile = 0;
	seek(0, 0, 0);
}

/*
 * Open next input file
 */
nextfile()
{
	if (++cfile >= nifiles)
		return(0);
	close(0);
	if (open(ifiles[cfile], 0) != 0) {
		printf("%s: can't open\n", ifiles[cfile]);
		exit(1);
	}
	line = 0;
	return(1);
}

/*
 * Print error message and continue
 */
error(type)
{
	if (nifiles > 1)
		printf("%s: ", ifiles[cfile]);
	printf("%d: %c\n", line, type);
	errcnt++;
}

/*
 * Print error message and abort current line
 */
xerror(type)
{
	error(type);
	nexttoken = 0;
	reset();
}
