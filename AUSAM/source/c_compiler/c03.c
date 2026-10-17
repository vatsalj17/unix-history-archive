#
/*
 * C compiler, phase 1
 *
 *
 * Handles processing of declarations,
 * except for top-level processing of
 * externals.
 */

#include "c0.h"

/*
 * Process a sequence of declaration statements
 */
declist(sclass, uf)
{
	register sc, offset;
	struct hshtab typer;

	offset = 0;
	bitoffs = 0;
	sc = sclass;
	while (getkeywords(&sclass, &typer)) {
		if(uf) 
			bitoffs = 0;
		offset = max(offset, declare(sclass, &typer, uf? 0: offset));
		sclass = sc;
	}
	return(offset+align(INT, offset, 0));
}

/*
 * Read the keywords introducing a declaration statement
 * Store back the storage class, and fill in the type
 * entry, which looks like a hash table entry.
 */
getkeywords(scptr, tptr)
int *scptr;
struct hshtab *tptr;
{
	register skw, tkw, longf;
	int o, isadecl, ismos, unsignf;

	isadecl = 0;
	longf = 0;
	unsignf = 0;
	tptr->htype = INT;
	tptr->hstrp = NULL;
	tptr->hsubsp = NULL;
	tkw = -1;
	skw = *scptr;
	ismos = skw==MOS;
	for (;;) {
		mosflg = ismos && isadecl;
		o = symbol();
		if (o==NAME && csym->hclass==TYPEDEF) {
			if (tkw>=0)
				error("Type clash");
			tkw = csym->htype;
			tptr->hsubsp = csym->hsubsp;
			tptr->hstrp = csym->hstrp;
			isadecl++;
			continue;
		}
		switch (o==KEYW? cval: -1) {
		case AUTO:
		case STATIC:
		case EXTERN:
		case REG:
		case TYPEDEF:
			if (skw && skw!=cval) {
				if (skw==ARG && cval==REG)
					cval = AREG;
				else
					error("Conflict in storage class");
			}
			skw = cval;
			break;
	
		case UNSIGN:
			unsignf++;
			break;

		case LONG:
			longf++;
			break;

		case UNION:
		case STRUCT:
			tptr->hstrp = strdec(ismos, cval);
			cval = STRUCT;
		case INT:
		case CHAR:
		case FLOAT:
		case DOUBLE:
			if (tkw>=0)
				error("Type clash");
			tkw = cval;
			break;
	
		default:
			peeksym = o;
			if (isadecl==0)
				return(0);
			if (tkw<0)
				tkw = INT;
			if (skw==0)
				skw = blklev==0? EXTERN: AUTO;
			if (unsignf) {
				if (tkw==INT)
					tkw = UNSIGN;
				else
					error("Misplaced 'unsigned'");
			}
			if (longf) {
				if (tkw==FLOAT)
					tkw = DOUBLE;
				else if (tkw==INT)
					tkw = LONG;
				else
					error("Misplaced 'long'");
			}
			*scptr = skw;
			tptr->htype = tkw;
			return(1);
		}
		isadecl++;
	}
}

/*
 * Process a structure declaration; a subroutine
 * of getkeywords.
 */
struct str *
strdec(mosf, uf)
{
	register elsize, o;
	register struct hshtab *ssym;
	int savebits;
	struct hshtab **savememlist;
	int savenmems;
	struct str *strp;
	struct hshtab *ds;
	struct hshtab *mems[NMEMS];

	mosflg = 1;
	ssym = 0;
	if ((o=symbol())==NAME) {
		ssym = csym;
		mosflg = mosf;
		o = symbol();
		if (o==LBRACE && ssym->hblklev<blklev)
			pushdecl(ssym);
		if (ssym->hclass==0) {
			ssym->hclass = STRTAG;
			ssym->strp = gblock(sizeof(*strp));
			funcbase = curbase;
			ssym->strp->ssize = 0;
			ssym->strp->memlist = NULL;
		}
		if (ssym->hclass != STRTAG)
			redec();
		strp = ssym->strp;
	} else {
		strp = gblock(sizeof(*strp));
		funcbase = curbase;
		strp->ssize = 0;
		strp->memlist = NULL;
	}
	mosflg = 0;
	if (o != LBRACE) {
		if (ssym==0)
			goto syntax;
		if (ssym->hclass!=STRTAG)
			error("Bad structure name");
		peeksym = o;
	} else {
		ds = defsym;
		savebits = bitoffs;
		savememlist = memlist;
		savenmems = nmems;
		memlist = mems;
		nmems = 2;
		bitoffs = 0;
		elsize = declist(MOS, uf-STRUCT);
		bitoffs = savebits;
		defsym = ds;
		if (strp->ssize)
			error("%.8s redeclared", ssym->name);
		strp->ssize = elsize;
		*memlist++ = NULL;
		strp->memlist = gblock((memlist-mems)*sizeof(*memlist));
		funcbase = curbase;
		for (o=0; &mems[o] != memlist; o++)
			strp->memlist[o] = mems[o];
		memlist = savememlist;
		nmems = savenmems;
		if ((o = symbol()) != RBRACE)
			goto syntax;
	}
	return(strp);
   syntax:
	decsyn(o);
	return(0);
}

/*
 * Process a comma-separated list of declarators
 */
declare(askw, tptr, offset)
struct hshtab *tptr;
{
	register int o;
	register int skw;

	skw = askw;
	do {
		offset =+ decl1(skw, tptr, offset);
	} while ((o=symbol()) == COMMA);
	if (o!=SEMI && (o!=RPARN || skw!=ARG1))
		decsyn(o);
	return(offset);
}

/*
 * Process a single declarator
 */
decl1(askw, atptr, offset)
struct hshtab *atptr;
{
	int t1, chkoff, a, elsize;
	register int skw;
	int type;
	register struct hshtab *dsym;
	register struct hshtab *tptr;
	struct tdim dim;
	struct field *fldp;
	int *dp;
	int isinit;

	skw = askw;
	tptr = atptr;
	chkoff = 0;
	mosflg = skw==MOS;
	dim.rank = 0;
	if (tptr->hsubsp) {
		t1 = tptr->htype;
		for (a=0; t1&XTYPE;) {
			if ((t1&XTYPE)==ARRAY) {
				dim.dimens[a] = tptr->hsubsp[a];
				a++;
				dim.rank++;
			}
			t1 =>> TYLEN;
		}
	}
	if ((peeksym=symbol())==SEMI || peeksym==RPARN)
		return(0);
	/*
	 * Filler field
	 */
	if (peeksym==COLON && skw==MOS) {
		peeksym = -1;
		t1 = conexp();
		elsize = align(tptr->htype, offset, t1);
		bitoffs =+ t1;
		return(elsize);
	}
	if ((t1=getype(&dim)) < 0)
		goto syntax;
	type = tptr->htype & ~TYPE;
	while (t1&XTYPE) {
		type = type<<TYLEN | (t1 & XTYPE);
		t1 =>> TYLEN;
	}
	type =| tptr->htype&TYPE;
	dsym = defsym;
	if (dsym->hblklev < blklev)
		pushdecl(dsym);
	if (dim.rank == 0)
		dsym->subsp = NULL;
	else {
		dp = gblock(dim.rank*sizeof(dim.rank));
		funcbase = curbase;
		for (a=0; a<dim.rank; a++) {
			if ((t1 = dp[a] = dim.dimens[a])
			 && (dsym->htype&XTYPE) == ARRAY
			 && dsym->subsp[a] && t1!=dsym->subsp[a])
				redec();
		}
		dsym->subsp = dp;
	}
	if ((type&XTYPE) == FUNC) {
		if (skw==AUTO)
			skw = EXTERN;
		if (skw!=EXTERN)
			error("Bad func. storage class");
	}
	if (!(dsym->hclass==0
	   || ((skw==ARG||skw==AREG) && dsym->hclass==ARG1)
	   || (skw==EXTERN && dsym->hclass==EXTERN && dsym->htype==type)))
		if (skw==MOS && dsym->hclass==MOS && dsym->htype==type)
			chkoff = 1;
		else {
			redec();
			goto syntax;
		}
	if (dsym->hclass && (dsym->htype&TYPE)==STRUCT && (type&TYPE)==STRUCT)
		if (dsym->hstrp != tptr->hstrp) {
			error("Warning: structure redeclaration");
			nerror--;
		}
	dsym->htype = type;
	if (tptr->hstrp)
		dsym->hstrp = tptr->hstrp;
	if (skw==TYPEDEF) {
		dsym->hclass = TYPEDEF;
		return(0);
	}
	if (skw==ARG1) {
		if (paraml==0)
			paraml = dsym;
		else
			parame->hoffset = dsym;
		parame = dsym;
		dsym->hclass = skw;
		return(0);
	}
	elsize = 0;
	if (skw==MOS) {
		elsize = length(dsym);
		if ((peeksym = symbol())==COLON) {
			elsize = 0;
			peeksym = -1;
			t1 = conexp();
			a = align(type, offset, t1);
			if (dsym->hflag&FFIELD) {
				if (dsym->hstrp->bitoffs!=bitoffs
			 	 || dsym->hstrp->flen!=t1)
					redec();
			} else {
				dsym->hstrp = gblock(sizeof(*fldp));
				funcbase = curbase;
			}
			dsym->hflag =| FFIELD;
			dsym->hstrp->bitoffs = bitoffs;
			dsym->hstrp->flen = t1;
			bitoffs =+ t1;
		} else
			a = align(type, offset, 0);
		elsize =+ a;
		offset =+ a;
		if (++nmems >= NMEMS) {
			error("Too many structure members");
			nmems =- NMEMS/2;
			memlist =- NMEMS/2;
		}
		if (a)
			*memlist++ = &structhole;
		if (chkoff && dsym->hoffset != offset)
			redec();
		dsym->hoffset = offset;
		*memlist++ = dsym;
	}
	if (skw==REG)
		if ((dsym->hoffset = goodreg(dsym)) < 0)
			skw = AUTO;
	dsym->hclass = skw;
	isinit = 0;
	if ((peeksym=symbol())!=COMMA && peeksym!=SEMI) {
		if (peeksym == ASSIGN)
			peeksym = -1;
		isinit++;
	}
	if (skw==AUTO) {
	/*	if (STAUTO < 0) {	*/
			autolen =- rlength(dsym);
			dsym->hoffset = autolen;
			if (autolen < maxauto)
				maxauto = autolen;
	/*	} else { 			*/
	/*		dsym->hoffset = autolen;	*/
	/*		autolen =+ rlength(dsym);	*/
	/*		if (autolen > maxauto)		*/
	/*			maxauto = autolen;	*/
	/*	}			*/
		if (isinit)
			cinit(dsym, 0, AUTO);
	} else if (skw==STATIC) {
		dsym->hoffset = isn;
		if (isinit) {
			outcode("BBN", DATA, LABEL, isn++);
			if (cinit(dsym, 1, STATIC) & ALIGN)
				outcode("B", EVEN);
		} else
			outcode("BBNBN", BSS, LABEL, isn++, SSPACE, rlength(dsym));
		outcode("B", PROG);
	} else if (skw==REG && isinit)
		cinit(dsym, 0, REG);
	prste(dsym);
syntax:
	return(elsize);
}

/*
 * Push down an outer-block declaration
 * after redeclaration in an inner block.
 */
pushdecl(asp)
struct phshtab *asp;
{
	register struct phshtab *sp, *nsp;

	sp = asp;
	nsp = gblock(sizeof(*nsp));
	maxdecl = funcbase = curbase;
	cpysymb(nsp, sp);
	sp->hclass = 0;
	sp->hflag =& ~(FNUND|FFIELD|FINIT|FLABL);
	sp->htype = 0;
	sp->hoffset = 0;
	sp->hblklev = blklev;
	sp->hpdown = nsp;
}

/*
 * Copy the non-name part of a symbol
 */
cpysymb(s1, s2)
struct phshtab *s1, *s2;
{
	register struct phshtab *rs1, *rs2;

	rs1 = s1;
	rs2 = s2;
	rs1->hclass = rs2->hclass;
	rs1->hflag = rs2->hflag;
	rs1->htype = rs2->htype;
	rs1->hoffset = rs2->hoffset;
	rs1->hsubsp = rs2->hsubsp;
	rs1->hstrp = rs2->hstrp;
	rs1->hblklev = rs2->hblklev;
	rs1->hpdown = rs2->hpdown;
}


/*
 * Read a declarator and get the implied type
 */
getype(adimp)
struct tdim *adimp;
{
	static struct hshtab argtype;
	int type;
	register int o;
	register struct hshtab *ds;
	register struct tdim *dimp;

	ds = defsym;
	dimp = adimp;
	switch(o=symbol()) {

	case TIMES:
		return(getype(dimp)<<TYLEN | PTR);

	case LPARN:
		type = getype(dimp);
		ds = defsym;
		if ((o=symbol()) != RPARN)
			goto syntax;
		goto getf;

	case NAME:
		defsym = ds = csym;
		type = 0;
	getf:
		switch(o=symbol()) {

		case LPARN:
			if (blklev==0) {
				blklev++;
				ds = defsym;
				declare(ARG1, &argtype, 0);
				defsym = ds;
				blklev--;
			} else
				if ((o=symbol()) != RPARN)
					goto syntax;
			type = type<<TYLEN | FUNC;
			goto getf;

		case LBRACK:
			if (dimp->rank>=5) {
				error("Rank too large");
				dimp->rank = 4;
			}
			if ((o=symbol()) != RBRACK) {
				peeksym = o;
				cval = conexp();
				if ((o=symbol())!=RBRACK)
					goto syntax;
			} else {
				if (dimp->rank!=0)
					error("Null dimension");
				cval = 0;
			}
			for (o=0; o < dimp->rank; o++)
				dimp->dimens[o] =* cval;
			dimp->dimens[dimp->rank++] = cval;
			type = type<<TYLEN | ARRAY;
			goto getf;
		}
		peeksym = o;
		return(type);
	}
syntax:
	decsyn(o);
	return(-1);
}

/*
 * Enforce alignment restrictions in structures,
 * including bit-field considerations.
 */
align(type, offset, aflen)
{
	register a, t, flen;
	char *ftl;

	flen = aflen;
	a = offset;
	t = type;
	ftl = "Field too long";
	if (flen==0) {
		a =+ (NBPC+bitoffs-1) / NBPC;
		bitoffs = 0;
	}
	while ((t&XTYPE)==ARRAY)
		t = decref(t);
	if (t!=CHAR) {
		a = (a+ALIGN) & ~ALIGN;
		if (a>offset)
			bitoffs = 0;
	}
	if (flen) {
		if (type==INT || type==UNSIGN) {
			if (flen > NBPW)
				error(ftl);
			if (flen+bitoffs > NBPW) {
				bitoffs = 0;
				a =+ NCPW;
			}
		} else if (type==CHAR) {
			if (flen > NBPC)
				error(ftl);
			if (flen+bitoffs > NBPC) {
				bitoffs = 0;
				a =+ 1;
			}
		} else
			error("Bad type for field");
	}
	return(a-offset);
}

/*
 * Complain about syntax error in declaration
 */
decsyn(o)
{
	error("Declaration syntax");
	errflush(o);
}

/*
 * Complain about a redeclaration
 */
redec()
{
	error("%.8s redeclared", defsym->name);
}

/*
 * Determine if a variable is suitable for storage in
 * a register; if so return the register number
 */
goodreg(hp)
register struct hshtab *hp;
{
	register int type;

	type = hp->htype;
	if ((type!=INT && type!=CHAR && type!=UNSIGN && (type&XTYPE)==0
			&& (type != STRUCT || rlength(hp)>2))
	 || (type&XTYPE)>PTR || regvar<3)
		return(-1);
	return(--regvar);
}
