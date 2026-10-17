/*
 *		Assembler symbol table management
 *
 *
 *	Copyright (C) 1978, Richard Miller
 */

#define EXTERN	extern
#include "as.h"

#define NSYM	100		/* initial no. of symbols */
#define SYMINC	100		/* increment for symtab expansion */
#define HASHSIZE 199		/* size of hashed index to symtab
				   should be prime for best distribution */

/*
 * Hashing function for symbol table index:
 *	defined as a macro for speed
 */
#define hash(name)	((((unsigned *)name)[0]+((unsigned *)name)[1])%HASHSIZE)

struct symbol *hashtab[HASHSIZE];	/* index to symbol table */

extern struct optab	opcodes[];		/* built-in opcodes */
extern struct symbol	predefs[];		/* pre-defined symbols */

/*
 * Initialize symbol table:
 *	- enter opcodes into hashed index
 *	- allocate initial space for user-defined symbols
 *	- initialize predefined symbols
 */
syminit()
{
	register struct symbol *sp, **hp;
	register struct optab *op;

	for (op = opcodes; op->name[0]; op++) {
		hp = &hashtab[hash(op->name)];
		op->next = *hp;
		*hp = op;
	}

	nextsym = usymtab = sbrk(0);
	brk(symtop = nextsym + NSYM);

	for (sp = predefs; sp->name[0]; sp++) {
		enter(sp->name);
		cursym->type = sp->type;
		cursym->value = sp->value;
	}
}

/*
 * Append symbol table to object file
 */
symout()
{
	register struct symbol *sp;
	register char c, *p;

	seek(ofile, symseek, 0);
	for (sp = usymtab; sp < nextsym; sp++) {
		/*
		 * relocate
		 */
		switch (sp->type&SSEG) {

		case SCOMN:
			/* change common block members to absolute */
			sp->type = SABS;
			break;

		case SBSS:
			sp->value += hdr.dsize;
		case SDATA:
			sp->value += hdr.tsize;

		}
		write(ofile, sp->name, 16);
	}
}

/*
 * Look up symbol name in symbuf in hash-indexed symbol table
 *	flag =	0:	opcode expected
 *		1:	user symbol expected
 *		2:	user symbol expected; enter into index if not found
 *
 *	returns	0:	symbol was not previously defined
 *		1:	symbol was previously defined
 *		cursym:	points to symbol table entry
 */
symlook(flag)
{
	register struct symbol *sym, *sp, *lp, **hp;

	lp = hp = &hashtab[hash(&symbuf)];

	for (sp = *hp; sp; lp = sp, sp = sp->next) {
		if (((int *)sp->name)[0] == ((int *)&symbuf)[0] &&
		    ((int *)sp->name)[1] == ((int *)&symbuf)[1]) {
			if (flag) {
				if (sp < usymtab)
					continue;
			} else
				if (sp >= usymtab)
					continue;
			/*
			 * Move symbol to the front of the chain so it will
			 * be found sooner next time.
			 */
			if (lp != hp) {
				lp->next = sp->next;
				sp->next = *hp;
				*hp = sp;
			}
			cursym = sp;
			return(1);
		}
	}

	/*
	 * Enter new symbol into symbol table and index
	 */
	if (flag > 1) {
		sym = nextsym;
		if (++nextsym > symtop)
			brk(symtop += SYMINC);
		((int *)sym->name)[0] = ((int *)&symbuf)[0];
		((int *)sym->name)[1] = ((int *)&symbuf)[1];
		sym->next = *hp;
		*hp = cursym = sym;
	}
	return(0);
}

/*
 * Enter a symbol name into the symbol table
 */
enter(name)
register char *name;
{
	register i;

	for (i=0; i<8; i++)
		if (symbuf[i] = *name)
			name++;
	symlook(2);
}
