/*
 * Routines to parse .u1 files and produce icode.
 */

#include "opcode.h"
#include "ilink.h"
#include "../h/keyword.h"
#include "../h/version.h"
#include "../h/header.h"

#ifndef MaxHeader
#define MaxHeader MaxHdr
#endif MaxHeader

static word pc = 0;		/* simulated program counter */

#define outword(n)	wordout((word)(n))

/*
 * gencode - read .u1 file, resolve variable references, and generate icode.
 *  Basic process is to read each line in the file and take some action
 *  as dictated by the opcode.  This action sometimes involves parsing
 *  of operands and usually culminates in the call of the appropriate
 *  emit* routine.
 *
 * Appendix C of the "tour" has a complete description of the intermediate
 *  language that gencode parses.
 */
gencode()
   {
   register int op, k, lab;
   int j, nargs, flags, implicit;
   char *id, *name, *procname;
   struct centry *cp;
   struct gentry *gp;
   struct fentry *fp, *flocate();

   extern long getint();
   extern double getreal();
   union {
      long ival;
      double rval;
      char *sval;
      } gg;
   extern char *getid(), *getstrlit();
   extern struct gentry *glocate();

   while ((op = getop(&name)) != EOF) {
      switch (op) {

         /* Ternary operators. */

         case Op_Toby:
         case Op_Sect:

         /* Binary operators. */

         case Op_Asgn:
         case Op_Cat:
         case Op_Diff:
         case Op_Div:
         case Op_Eqv:
         case Op_Inter:
         case Op_Lconcat:
         case Op_Lexeq:
         case Op_Lexge:
         case Op_Lexgt:
         case Op_Lexle:
         case Op_Lexlt:
         case Op_Lexne:
         case Op_Minus:
         case Op_Mod:
         case Op_Mult:
         case Op_Neqv:
         case Op_Numeq:
         case Op_Numge:
         case Op_Numgt:
         case Op_Numle:
         case Op_Numlt:
         case Op_Numne:
         case Op_Plus:
         case Op_Power:
         case Op_Rasgn:
         case Op_Rswap:
         case Op_Subsc:
         case Op_Swap:
         case Op_Unions:

         /* Unary operators. */

         case Op_Bang:
         case Op_Compl:
         case Op_Neg:
         case Op_Nonnull:
         case Op_Null:
         case Op_Number:
         case Op_Random:
         case Op_Refresh:
         case Op_Size:
         case Op_Tabmat:
         case Op_Value:

         /* Instructions. */

         case Op_Bscan:
         case Op_Ccase:
         case Op_Coact:
         case Op_Cofail:
         case Op_Coret:
         case Op_Dup:
         case Op_Efail:
         case Op_Eret:
         case Op_Escan:
         case Op_Esusp:
         case Op_Limit:
         case Op_Lsusp:
         case Op_Pfail:
         case Op_Pnull:
         case Op_Pop:
         case Op_Pret:
         case Op_Psusp:
         case Op_Push1:
         case Op_Pushn1:
         case Op_Sdup:
            newline();
            emit(op, name);
            break;

         case Op_Chfail:
         case Op_Create:
         case Op_Goto:
         case Op_Init:
            lab = getlab();
            newline();
            emitl(op, lab, name);
            break;

         case Op_Cset:
         case Op_Real:
            k = getdec();
            newline();
            emitr(op, ctable[k].c_pc, name);
            break;

         case Op_Field:
            id = getid();
            newline();
            fp = flocate(id);
            if (fp == NULL) {
               err(id, "invalid field name", 0);
               break;
               }
            emitn(op, (word)(fp->f_fid-1), name);
            break;

         case Op_Int:
            k = getdec();
            newline();
            cp = &ctable[k];
            if (cp->c_flag & F_LongLit)
               emitr(Op_Long, cp->c_pc, "long");
            else {
               long int i;
               i = (long)cp->c_val.ival;
               emitint(op, i, name);
               }
            break;

         case Op_Invoke:
            k = getdec();
            newline();
            emitn(op, (word)k, name);
            break;

         case Op_Keywd:
            k = getdec();
            newline();
            switch (k) {
               case K_FAIL:
                  emit(Op_Efail,"efail");
                  break;
               case K_NULL:
                  emit(Op_Pnull,"pnull");
                  break;
               default:
               emitn(op, (word)k, name);
            }
            break;

         case Op_Llist:
            k = getdec();
            newline();
            emitn(op, (word)k, name);
            break;

         case Op_Lab:
            lab = getlab();
            newline();
            if (Dflag)
               fprintf(dbgfile, "L%d:\n", lab);
            backpatch(lab);
            break;

         case Op_Line:
            line = getdec();
            newline();
            emitn(op, (word)line, name);
            break;

         case Op_Mark:
            lab = getlab();
            newline();
            emitl(op, lab, name);
            break;

         case Op_Mark0:
            emit(op, name);
            break;

         case Op_Str:
            k = getdec();
            newline();
            cp = &ctable[k];
            id = cp->c_val.sval;
            emitin(op, (word)(id-strings), cp->c_length, name);
            break;
	
         case Op_Tally:
            k = getdec();
            newline();
            emitn(op, (word)k, name);
            break;

         case Op_Unmark:
            emit(Op_Unmark, name);
            break;

         case Op_Var:
            k = getdec();
            newline();
            flags = ltable[k].l_flag;
            if (flags & F_Global)
               emitn(Op_Global, (word)(ltable[k].l_val.global-gtable), "global");
            else if (flags & F_Static)
               emitn(Op_Static, (word)(ltable[k].l_val.staticid-1), "static");
            else if (flags & F_Argument)
               emitn(Op_Arg, (word)(ltable[k].l_val.offset-1), "arg");
            else
               emitn(Op_Local, (word)(ltable[k].l_val.offset-1), "local");
            break;

         /* Declarations. */

         case Op_Proc:
            procname = getid();
            newline();
            locinit();
            clearlab();
            line = 0;
            gp = glocate(procname);
            implicit = gp->g_flag & F_ImpError;
            nargs = gp->g_nargs;
            emiteven();
            break;

         case Op_Local:
            k = getdec();
            flags = getoct();
            id = getid();
            putloc(k, id, flags, implicit, procname);
            break;

         case Op_Con:
            k = getdec();
            flags = getoct();
            if (flags & F_IntLit) {
               gg.ival = getint();
               putconst(k, flags, 0, pc, gg);
               }
            else if (flags & F_RealLit) {
               gg.rval = getreal();
               putconst(k, flags, 0, pc, gg);
               }
            else if (flags & F_StrLit) {
               j = getdec();
               gg.sval = getstrlit(j);
               putconst(k, flags, j, pc, gg);
               }
            else if (flags & F_CsetLit) {
               j = getdec();
               gg.sval = getstrlit(j);
               putconst(k, flags, j, pc, gg);
               }
            else
               fprintf(stderr, "gencode: illegal constant\n");
            newline();
            emitcon(k);
            break;

         case Op_Filen:
            file = getid();
            newline();
            break;

         case Op_Declend:
            newline();
            gp->g_pc = pc;
            emitproc(procname, nargs, dynoff, statics-static1, static1);
            break;

         case Op_End:
            newline();
            flushcode();
            break;

         default:
            fprintf(stderr, "gencode: illegal opcode(%d): %s\n", op, name);
            newline();
         }
      }
   }

/*
 *  emit - emit opcode.
 *  emitl - emit opcode with reference to program label, consult the "tour"
 *	for a description of the chaining and backpatching for labels.
 *  emitn - emit opcode with integer argument.
 *  emitr - emit opcode with pc-relative reference.
 *  emiti - emit opcode with reference to identifier table.
 *  emitin - emit opcode with reference to identifier table & integer argument.
 *  emitint - emit word opcode with integer argument.
 *  emiteven - emit null bytes to bring pc to word boundary.
 *  emitcon - emit constant table entry.
 *  emitproc - emit procedure block.
 *
 * The emit* routines call out* routines to effect the "outputting" of icode.
 *  Note that the majority of the code for the emit* routines is for debugging
 *  purposes.
 */
emit(op, name)
int op;
char *name;
   {
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\t\t\t\t# %s\n", (long)pc, op, name);
   outword(op);
   }

emitl(op, lab, name)
int op, lab;
char *name;
   {
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\tL%d\t\t\t# %s\n", (long)pc, op, lab, name);
   if (lab >= maxlabels)
      syserr("too many labels in ucode");
   outword(op);
   if (labels[lab] <= 0) {		/* forward reference */
      outword(labels[lab]);
      labels[lab] = WordSize - pc;	/* add to front of reference chain */
      }
   else					/* output relative offset */
      outword(labels[lab] - (pc + WordSize));
   }

emitn(op, n, name)
int op;
word n;
char *name;
   {
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\t%ld\t\t\t# %s\n", (long)pc, op, (long)n, name);
   outword(op);
   outword(n);
   }

emitr(op, loc, name)
int op;
word loc;
char *name;
   {
   loc -= pc + (WordSize * 2);
   if (Dflag) {
      if (loc >= 0)
	 fprintf(dbgfile, "%ld:\t%d\t*+%ld\t\t\t# %s\n",(long) pc, op, (long)loc, name);
      else
	 fprintf(dbgfile, "%ld:\t%d\t*-%ld\t\t\t# %s\n",(long) pc, op, (long)-loc, name);
      }
   outword(op);
   outword(loc);
   }

emiti(op, offset, name)
int op;
word offset;
char *name;
   {
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\tI+%d\t\t\t# %s\n", (long)pc, op, offset, name);
   outword(op);
   outword(offset);
   }

emitin(op, offset, n, name)
int op, n;
word offset;
char *name;
   {
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\t%d,I+%ld\t\t\t# %s\n", (long)pc, op, n, (long)offset, name);
   outword(op);
   outword(n);
   outword(offset);
   }
/*
 * emitint can have some pitfalls.  outword is used to output the
 *  integer and this is picked up in the interpreter as the second
 *  word of a short integer.  The integer value output must be
 *  the same size as what the interpreter expects.  See op_int and op_intx
 *  in interp.s
 */
emitint(op, i, name)
int op;
long int i;
char *name;
   {
   if (Dflag)
	fprintf(dbgfile, "%ld:\t%d\t%ld\t\t\t# %s\n", (long)pc, op, (long)i, name);
   outword(op);
   outword(i);
   }

emiteven()
   {
   while ((pc % WordSize) != 0) {
      if (Dflag)
	 fprintf(dbgfile, "%ld:\t0\n",(long) pc);
      outword(0);
      }
   }

emitcon(k)
register int k;
   {
   register int i, j;
   register char *s;
   int csbuf[CsetSize];
   union {
      char ovly[1];  /* Array used to overlay l and f on a bytewise basis. */
      long int l;
      double f;
      } x;

   if (ctable[k].c_flag & F_RealLit) {
#ifdef Double
/* access real values one word at a time */
      {  int *rp, *rq;
	 rp = (int *) &(x.f);
	 rq = (int *) &(ctable[k].c_val.rval);
	 *rp++ = *rq++;
	 *rp   = *rq;
      }
#else Double
      x.f = ctable[k].c_val.rval;
#endif Double
      if (Dflag) {
	 fprintf(dbgfile, "%ld:\t%d\n", (long)pc, T_Real);
	 dumpblock(x.ovly,sizeof(double));
	 fprintf(dbgfile, "\t\t\t( %g )\n",x.f);
	 }
      outword(T_Real);

#ifdef Double
/* fill out real block with an empty word */
      outword(0);
#endif Double
      outblock(x.ovly,sizeof(double));
      }
   else if (ctable[k].c_flag & F_LongLit) {
      x.l = ctable[k].c_val.ival;
      if (Dflag) {
	 fprintf(dbgfile, "%ld:\t%d\n",(long) pc, T_Longint);
	 dumpblock(x.ovly,sizeof(long));
	 fprintf(dbgfile,"\t\t\t( %ld)\n",(long)x.l);
	 }
      outword(T_Longint);
      outblock(x.ovly,sizeof(long));
      }
   else if (ctable[k].c_flag & F_CsetLit) {
      for (i = 0; i < CsetSize; i++)
	 csbuf[i] = 0;
      s = ctable[k].c_val.sval;
      i = ctable[k].c_length;
      while (i--) {
	 Setb(*s, csbuf);
	 s++;
	 }
      j = 0;
      for (i = 0; i < 256; i++) {
	 if (Testb(i, csbuf))
	   j++;
	 }
      if (Dflag) {
	 fprintf(dbgfile, "%ld:\t%d\n",(long) pc, T_Cset);
	 fprintf(dbgfile, "\t%d\n",j);
	 fprintf(dbgfile,csbuf,sizeof(csbuf));
	 }
      outword(T_Cset);
      outword(j);		   /* cset size */
      outblock(csbuf,sizeof(csbuf));
      if (Dflag)
	 dumpblock(csbuf,CsetSize);
      }
   }

emitproc(name, nargs, ndyn, nstat, fstat)
char *name;
int nargs, ndyn, nstat, fstat;
   {
   register int i;
   register char *p;
   int size;
   /*
    * FncBlockSize = sizeof(BasicFncBlock) +
    *  sizeof(descrip)*(# of args + # of dynamics + # of statics).
    */
   size = (10*WordSize) + (2*WordSize) * (nargs+ndyn+nstat);

   if (Dflag) {
      fprintf(dbgfile, "%ld:\t%d\n", (long)pc, T_Proc); /* type code */
      fprintf(dbgfile, "\t%d\n", size);                 /* size of block */
      fprintf(dbgfile, "\tZ+%ld\n",(long)(pc+size));    /* entry point */
      fprintf(dbgfile, "\t%d\n", nargs);                /* # of arguments */
      fprintf(dbgfile, "\t%d\n", ndyn);                 /* # of dynamic locals */
      fprintf(dbgfile, "\t%d\n", nstat);                /* # of static locals */
      fprintf(dbgfile, "\t%d\n", fstat);                /* first static */
      fprintf(dbgfile, "\t%s\n", file);                 /* file */
      fprintf(dbgfile, "\t%d\tI+%ld\t\t\t# %s\n",        /* name of procedure */
	 strlen(name), (long)(name-strings), name);
      }
   outword(T_Proc);
   outword(size);
   outword(pc + size - 2*WordSize); /* Have to allow for the two words
				     that we've already output. */
   outword(nargs);
   outword(ndyn);
   outword(nstat);
   outword(fstat);
   outword(file - strings);
   outword(strlen(name));
   outword(name - strings);

   /*
    * Output string descriptors for argument names by looping through
    *  all locals, and picking out those with F_Argument set.
    */
   for (i = 0; i <= nlocal; i++) {
      if (ltable[i].l_flag & F_Argument) {
	 p = ltable[i].l_name;
	 if (Dflag)
	    fprintf(dbgfile, "\t%d\tI+%ld\t\t\t# %s\n", strlen(p), (long)(p-strings), p);
	 outword(strlen(p));
	 outword(p - strings);
	 }
      }

   /*
    * Output string descriptors for local variable names.
    */
   for (i = 0; i <= nlocal; i++) {
      if (ltable[i].l_flag & F_Dynamic) {
	 p = ltable[i].l_name;
	 if (Dflag)
	    fprintf(dbgfile, "\t%d\tI+%ld\t\t\t# %s\n", strlen(p), (long)(p-strings), p);
	 outword(strlen(p));
	 outword(p - strings);
	 }
      }

   /*
    * Output string descriptors for local variable names.
    */
   for (i = 0; i <= nlocal; i++) {
      if (ltable[i].l_flag & F_Static) {
	 p = ltable[i].l_name;
	 if (Dflag)
	    fprintf(dbgfile, "\t%d\tI+%ld\t\t\t# %s\n", strlen(p), (long)(p-strings), p);
	 outword(strlen(p));
	 outword(p - strings);
	 }
      }
   }

/*
 * gentables - generate interpreter code for global, static,
 *  identifier, and record tables, and built-in procedure blocks.
 */

gentables()
   {
   register int i;
   register char *s;
   register struct gentry *gp;
   struct fentry *fp;
   struct rentry *rp;
   struct header hdr;
   char *strcpy();

   emiteven();

   /*
    * Output record constructor procedure blocks.
    */
   hdr.records = pc;
   if (Dflag)
      fprintf(dbgfile, "%ld:\t%d\t\t\t\t# record blocks\n",(long)pc, nrecords);
   outword(nrecords);
   for (gp = gtable; gp < gfree; gp++) {
      if (gp->g_flag & (F_Record & ~F_Global)) {
	 s = gp->g_name;
	 gp->g_pc = pc;
	 if (Dflag) {
	    fprintf(dbgfile, "%ld:\n", pc);
	    fprintf(dbgfile, "\t%d\n", T_Proc);
	    fprintf(dbgfile, "\t%d\n", RkBlkSize);
	    fprintf(dbgfile, "\t_mkrec\n");
	    fprintf(dbgfile, "\t%d\n", gp->g_nargs);
	    fprintf(dbgfile, "\t-2\n");
	    fprintf(dbgfile, "\t%d\n", gp->g_procid);
	    fprintf(dbgfile, "\t0\n");
	    fprintf(dbgfile, "\t0\n");
	    fprintf(dbgfile, "\t%d\tI+%ld\t\t\t# %s\n", strlen(s), (long)(s-strings), s);
	    }
	 outword(T_Proc);		/* type code */
	 outword(RkBlkSize);		/* size of block */
	 outword(0);			/* entry point (filled in by interp)*/
	 outword(gp->g_nargs);		/* number of fields */
	 outword(-2);			/* record constructor indicator */
	 outword(gp->g_procid); 	/* record id */
	 outword(0);			/* not used */
	 outword(0);			/* not used */
	 outword(strlen(s));		/* name of record */
	 outword(s - strings);
	 }
      }

   /*
    * Output record/field table.
    */
   hdr.ftab = pc;
   if (Dflag)
      fprintf(dbgfile, "%ld:\t\t\t\t\t# record/field table\n", (long)pc);
   for (fp = ftable; fp < ffree; fp++) {
      if (Dflag)
	 fprintf(dbgfile, "%ld:\n", (long)pc);
      rp = fp->f_rlist;
      for (i = 1; i <= nrecords; i++) {
	 if (rp != NULL && rp->r_recid == i) {
	    if (Dflag)
	       fprintf(dbgfile, "\t%d\n", rp->r_fnum);
	    outword(rp->r_fnum);
	    rp = rp->r_link;
	    }
	 else {
	    if (Dflag)
	       fprintf(dbgfile, "\t-1\n");
	    outword(-1);
	    }
	 if (Dflag && (i == nrecords || (i & 03) == 0))
	    putc('\n', dbgfile);
	 }
      }

   /*
    * Output global variable descriptors.
    */
   hdr.globals = pc;
   for (gp = gtable; gp < gfree; gp++) {
      if (gp->g_flag & (F_Builtin & ~F_Global)) {	/* built-in procedure */
	 if (Dflag)
	    fprintf(dbgfile, "%ld:\t%06lo\t%d\t\t\t# %s\n",
	       (long)pc, (long)D_Proc, -gp->g_procid, gp->g_name);
	 outword(D_Proc);
	 outword(-gp->g_procid);
	 }
      else if (gp->g_flag & (F_Proc & ~F_Global)) {	/* Icon procedure */
	 if (Dflag)
	    fprintf(dbgfile, "%ld:\t%06lo\tZ+%ld\t\t\t# %s\n",
	       (long)pc,(long)D_Proc, (long)gp->g_pc, gp->g_name);
	 outword(D_Proc);
	 outword(gp->g_pc);
	 }
      else if (gp->g_flag & (F_Record & ~F_Global)) {	/* record constructor */
	 if (Dflag)
	    fprintf(dbgfile, "%ld:\t%06lo\tZ+%ld\t\t\t# %s\n",
	       (long)pc, (long)D_Proc, (long)gp->g_pc, gp->g_name);
	 outword(D_Proc);
	 outword(gp->g_pc);
	 }
      else {	/* global variable */
	 if (Dflag)
	    fprintf(dbgfile, "%ld:\t%06lo\t0\t\t\t# %s\n",(long)pc,(long)D_Null, gp->g_name);
	 outword(D_Null);
	 outword(0);
	 }
      }

   /*
    * Output descriptors for global variable names.
    */
   hdr.gnames = pc;
   for (gp = gtable; gp < gfree; gp++) {
      if (Dflag)
	 fprintf(dbgfile, "%ld:\t%d\tI+%ld\t\t\t# %s\n",
		 (long)pc, strlen(gp->g_name), (long)(gp->g_name-strings), gp->g_name);
      outword(strlen(gp->g_name));
      outword(gp->g_name - strings);
      }

   /*
    * Output a null descriptor for each static variable.
    */
   hdr.statics = pc;
   for (i = statics; i > 0; i--) {
      if (Dflag)
	 fprintf(dbgfile, "%ld:\t0\t0\n", (long)pc);
      outword(D_Null);
      outword(0);
      }
   flushcode();

   /*
    * Output the identifier table.  Note that the call to write
    *  really does all the work.
    */
   hdr.ident = pc;
   if (Dflag) {
      for (s = strings; s < strfree; ) {
	 fprintf(dbgfile, "%ld:\t%03o\n", (long)pc, *s++);
	 for (i = 7; i > 0; i--) {
	    if (s >= strfree)
	       break;
	    fprintf(dbgfile, " %03o\n", *s++);
	    }
	 putc('\n', dbgfile);
	 }
      }
#ifndef MSDOS
   write(fileno(outfile), strings, strfree - strings);
#else MSDOS
#ifdef SPTR
   write(fileno(outfile), strings, strfree - strings);
#else  /* Handle the case where strfree-strings will create a long int */
   longwrite(fileno(outfile), strings, (long)(strfree - strings));
#endif
#endif MSDOS
   pc += strfree - strings;

   /*
    * Output icode file header.
    */
   hdr.hsize = pc;
   strcpy((char *)hdr.config,IVersion);
   hdr.trace = trace;
   if (Dflag) {
      fprintf(dbgfile, "size:    %ld\n", (long)hdr.hsize);
      fprintf(dbgfile, "trace:   %ld\n", (long)hdr.trace);
      fprintf(dbgfile, "records: %ld\n", (long)hdr.records);
      fprintf(dbgfile, "ftab:    %ld\n", (long)hdr.ftab);
      fprintf(dbgfile, "globals: %ld\n", (long)hdr.globals);
      fprintf(dbgfile, "gnames:  %ld\n", (long)hdr.gnames);
      fprintf(dbgfile, "statics: %ld\n", (long)hdr.statics);
      fprintf(dbgfile, "ident:   %ld\n", (long)hdr.ident);
      fprintf(dbgfile, "config:   %s\n", hdr.config);
      }
#ifndef NoHeader
   fseek(outfile, (long)MaxHeader, 0);
#else NoHeader
   fseek(outfile, 0L, 0);
#endif NoHeader
   write(fileno(outfile), &hdr, sizeof hdr);
   }

#define CodeCheck if (codep >= code + maxcode)\
		     syserr("out of code buffer space")
/*
 * outword(i) outputs i as a word that is used by the runtime system
 *  WordSize bytes must be moved from &word[0] to &codep[0].
 */
wordout(oword)
word oword;
   {
   int i;
   union {
	word i;
	char c[WordSize];
	} u;

   CodeCheck;
   u.i = oword;

   for (i = 0; i < WordSize; i++)
      codep[i] = u.c[i];

   codep += WordSize;
   pc += WordSize;
   }
/*
 * outblock(a,i) output i bytes starting at address a.
 */
outblock(addr,count)
char *addr;
int count;
   {
   if (codep + count > code + maxcode)
      syserr("out of code buffer space");
   pc += count;
   while (count--)
      *codep++ = *addr++;
   }
/*
 * dumpblock(a,i) dump contents of i bytes at address a, used only
 *  in conjunction with -D.
 */
dumpblock(addr, count)
char *addr;
int count;
   {
   int i;
   for (i = 0; i < count; i++) {
      if ((i & 7) == 0)
	 fprintf(dbgfile,"\n\t");
      fprintf(dbgfile," %03o\n",(unsigned)addr[i]);
      }
   putc('\n',dbgfile);
   }

/*
 * flushcode - write buffered code to the output file.
 */
flushcode()
   {
   if (codep > code)
      write(fileno(outfile), code, codep - code);
   codep = code;
   }

/*
 * clearlab - clear label table to all zeroes.
 */
clearlab()
   {
   register int i;

   for (i = 0; i < maxlabels; i++)
      labels[i] = 0;
   }

/*
 * backpatch - fill in all forward references to lab.
 */
backpatch(lab)
int lab;
   {
   word p, r;
   register char *q;
   register char *cp, *cr;
   register int j;

   if (lab >= maxlabels)
      syserr("too many labels in ucode");

   p = labels[lab];
   if (p > 0)
      syserr("multiply defined label in ucode");
   while (p < 0) {		/* follow reference chain */
      r = pc - (WordSize - p);	/* compute relative offset */
      q = codep - (pc + p);	/* point to word with address */
      cp = (char *) &p; 	/* address of integer p       */
      cr = (char *) &r; 	/* address of integer r       */
      for (j = 0; j < WordSize; j++) {	  /* move bytes from int pointed to */
	 *cp++ = *q;			  /* by q to p, and move bytes from */
	 *q++ = *cr++;			  /* r to int pointed to by q */
	 }			/* moves integers at arbitrary addresses */
      }
   labels[lab] = pc;
   }
#ifdef MSDOS
#ifdef LPTR
/* Write a long string in 32k chunks */
static longwrite(file,s,len)
int file;
char *s;
long int len;
{
   long int loopnum;
   unsigned int leftover;
   char *p;

   loopnum = len / 32768;
   leftover = len % 32768;
   for(p = s, loopnum = len/32768;loopnum;loopnum--) {
       write(file,p,32768);
       p += 32768;
   }
   if(leftover) write(file,p,leftover);
}
#endif LPTR
#endif MSDOS
