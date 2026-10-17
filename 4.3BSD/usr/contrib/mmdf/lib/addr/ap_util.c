#include "util.h"
#include "ap.h"

#if DEBUG > 1
extern char debug;
extern char *typtab[];
#endif

/*  Standard routines for handling address list element nodes

/*  < 1978  B. Borden       Wrote initial version of parser code
 *  78-80   D. Crocker      Reworked parser into current form
 *  Apr 81  K. Harrenstein  Hacked for SRI
 *  Jun 81  D. Crocker      Back in the fold.  Finished v7 conversion
 *                          minor cleanups.
 *                          repackaging into more complete set of calls
 *  Jul 81  D. Crocker      ap_free & _alloc check for not 0 or -1
 *                          malloc() error causes jump to ap_init error
 */

struct ap_prevstruct   *ap_fle;   /* parse state top of stack           */
				  /* "fl" => file, but could be other   */

int     (*ap_gfunc) ();           /* Ptr to character get fn            */

extern int      ap_peek;                  /* basic parse state info       */
extern int	ap_perlev;
extern int	ap_grplev;

extern char	*malloc();

/* ********************  LIST NODE PRIMITIVES  *********************** */

AP_ptr
	ap_alloc ()               /* create node, return pointer to it    */
{
    AP_ptr ap;

    /* NOSTRICT */
    ap = (AP_ptr) malloc (sizeof (struct ap_node));
    if (ap == (AP_ptr) 0)
	return ((AP_ptr) 0);

    ap_ninit (ap);
    return (ap);
}

ap_ninit (ap)
    register AP_ptr ap;
{
    ap -> ap_obtype = APV_NIL;
    ap -> ap_obvalue = (char *) 0;
    ap -> ap_ptrtype = APP_NIL;
    ap -> ap_chain = (AP_ptr) 0;
}

ap_free (ap)                      /* free node's storage                */
register AP_ptr ap;
{
    switch ((int)ap)
    {                             /* get rid of node, if have one       */
	case OK:
	case NOTOK:               /* nothing to free                    */
	    break;

	default:                  /* actually have a node               */
	    switch ((int)(ap -> ap_obvalue))
	    {                     /* get rid of its data string         */
		case OK:
		case NOTOK:       /* nothing to free                    */
		    break;

		default:
		    free (ap -> ap_obvalue);
	    }
	    free ((char *) ap);
    }
}

ap_fllnode (ap, obtype, obvalue)     /* add data to node at end of chain     */
register AP_ptr ap;
char   obtype;
register char  *obvalue;
{
    extern char *strdup ();

    ap -> ap_obtype = obtype;
    ap -> ap_obvalue = (obvalue == 0) ? (char *)0 : strdup (obvalue);

#if DEBUG > 1
    if (debug)
	printf ("(%s/'%s')", typtab[obtype], obvalue);
#endif
}

AP_ptr
	ap_new (obtype, obvalue)  /* alloc & fill node                  */
char    obtype;
char   *obvalue;
{
    register AP_ptr nap;

    nap = ap_alloc ();
    ap_fllnode (nap, obtype, obvalue);
    return (nap);
}

/* ***************  LIST MANIPULATION PRIMITIVES  ******************* */

ap_insert (cur, ptrtype, new)     /* create/fill/insert node in list    */
register AP_ptr cur;              /* where to insert after              */
char ptrtype;                     /* inserted is more or new address    */
register AP_ptr new;              /* where to insert after              */
{
 /* Now copy linkages from current node */

    new -> ap_ptrtype = cur -> ap_ptrtype;
    new -> ap_chain = cur -> ap_chain;

 /* Now point current node at inserted node */

    cur -> ap_ptrtype = ptrtype;
    cur -> ap_chain = new;
}

AP_ptr
	ap_sqinsert (cur, type, new)    /* insert sequence */
    register AP_ptr cur,
		    new;
    int type;
{
    AP_ptr oldptr;
    int otype;

    switch ((int)new) {
	case OK:
	case NOTOK:
	    return ((AP_ptr) 0);
    }

    oldptr = cur -> ap_chain;
    otype = cur -> ap_ptrtype;
    cur -> ap_chain = new;
    cur -> ap_ptrtype = type;

    while (new -> ap_ptrtype != APP_NIL &&
		new -> ap_chain != (AP_ptr) 0 &&
		new -> ap_chain -> ap_obtype != APV_NIL)
	new = new -> ap_chain;

    if (new -> ap_chain != (AP_ptr)0 && new -> ap_chain -> ap_obtype == APV_NIL)
	ap_delete (new);

    new -> ap_chain = oldptr;
    new -> ap_ptrtype = otype;
    return (new);
}

ap_delete (ap)                    /* remove next node in sequence       */
register AP_ptr ap;
{
    register AP_ptr next;

    if (ap != (AP_ptr) 0 && ap -> ap_ptrtype != APP_NIL)
    {                             /* only if there is something there   */
	next = ap -> ap_chain;    /* link around one to be removed      */

	ap -> ap_ptrtype = next -> ap_ptrtype;
	ap -> ap_chain = next -> ap_chain;

	ap_free (next);
    }
}

AP_ptr
	ap_append (ap, obtype, obvalue)
				  /* alloc, fill, insert node           */
register AP_ptr ap;               /* node to insert after               */
char    obtype;
char   *obvalue;
{
    register AP_ptr nap;

    nap = ap_alloc ();
    ap_fllnode (nap, obtype, obvalue);
    ap_insert (ap, APP_ETC, nap);
    return (nap);
}
/**/

AP_ptr
       ap_add (ap, obtype, obvalue)
				  /* try to append data to current node   */
register AP_ptr ap;
char    obtype;
register char  *obvalue;
{
    extern char *multcat ();
    register char  *ovalue;

    if (ap -> ap_obtype != obtype)
	return (ap_append (ap, obtype, obvalue));
    else                          /* same type or empty => can append     */
    {
	if (obvalue == 0)         /* No data to add                       */
	    return (OK);

	if ((ovalue = ap -> ap_obvalue) == (char *) 0)
	    ap_fllnode (ap, obtype, obvalue);
	else                      /* add to existing data                 */
	{
	    ovalue = ap -> ap_obvalue;
	    ap -> ap_obvalue = multcat (ovalue, " ", obvalue, (char *)0);
	    free (ovalue);
	}

#if DEBUG > 1
	if (debug)
	    printf ("+%d/'%s')", obtype, obvalue);
#endif
    }
    return (OK);
}

/**/

AP_ptr
	ap_sqdelete (strt_node, end_node) /* remove nodes, through end node     */
register AP_ptr strt_node;
register AP_ptr end_node;
{
    switch ((int)strt_node) {
	case OK:
	case NOTOK:
	    return ((AP_ptr)0);
    }
    while (strt_node -> ap_ptrtype != APP_NIL) {
	if (strt_node -> ap_chain == end_node) {
				/* last one requested                 */
	    ap_delete (strt_node);
	    return (strt_node -> ap_chain);
	}
	ap_delete (strt_node);
    }
    return ((AP_ptr) 0);          /* end of chain                       */
}

AP_ptr
	ap_1delete (ap)                   /* remove all nodes of address to NXT */
register AP_ptr ap;               /* starting node                      */
{
    while (ap -> ap_ptrtype != APP_NIL) {
	if (ap -> ap_ptrtype == APP_NXT)
	    return (ap -> ap_chain);
	ap_delete (ap);
    }
    return ((AP_ptr) 0);                   /* end of chain              */
}

ap_sqtfix (strt, end, obtype)     /* alter obtype of a node subsequence */
register AP_ptr strt;
register AP_ptr end;
register char   obtype;
{
    for ( ; ; strt = strt -> ap_chain) {
	if (strt -> ap_obtype != APV_CMNT)
	    strt -> ap_obtype = obtype;
	if (strt == end || strt -> ap_ptrtype == APP_NIL)
	    break;
    }
}

/**/

AP_ptr
	ap_move (to, from)  /* move node after from to be after to   */
    register AP_ptr to,
		    from;
{
    register AP_ptr nodeptr;

    if (from -> ap_ptrtype == APP_NIL || from -> ap_chain == (AP_ptr) 0)
	return (from);  /* quiet failure */

    nodeptr = from -> ap_chain;

    from -> ap_chain = nodeptr -> ap_chain;
    from -> ap_ptrtype = nodeptr -> ap_ptrtype;

    ap_insert (to, APP_ETC, nodeptr);
    return (from);      /* next in chain, now */
}

AP_ptr
	ap_sqmove (to, from, endtype)    /* move sequence */
    register AP_ptr to,
		    from;
    register char endtype;              /* copy only COMMENT and this */
{
    switch ((int)from) {
	case OK:
	case NOTOK:
	    return ((AP_ptr) 0);
    }

    while (from -> ap_ptrtype != APP_NIL && from -> ap_chain != (AP_ptr) 0) {
	if (endtype != (char) APV_NIL)
	    if (from -> ap_obtype != APV_CMNT && from -> ap_obtype != endtype)
		break;
	to = ap_move (to, from);
    }

    return (to);          /* end of chain                       */
}

/* ************************  PARSE STATE  *************************** */

AP_ptr ap_pstrt,                  /* current last node in parse tree    */
       ap_pcur;                   /* current last node in parse tree    */

ap_iinit (gfunc)                   /* input function initialization     */
int     (*gfunc) ();
{
    ap_gfunc = gfunc;             /* Set character fetch func           */
    ap_peek = -1;                 /* No lex peek char                   */
}

ap_clear ()                     /* Clear out the parser state           */
{
    ap_grplev = 0;                 /* Zero group nesting depth          */
    ap_perlev = 0;                 /* Zero <> nesting depth             */
}

AP_ptr
       ap_pinit (gfunc)           /* init, alloc & set start node       */
int     (*gfunc) ();
{
    ap_iinit (gfunc);
    return (ap_pstrt = ap_pcur = ap_alloc ());
}

/*  parse state saving uses a linked list of state information,
 *  recorded in ap_prevstruct structures.
 *  the list is manipulated as a simple stack.
 */

ap_ppush (gfunc)                  /* save parse context, ap_iinit    */
int     (*gfunc) ();
{
    extern char *malloc ();
    register struct ap_prevstruct  *tfil;

    /*NOSTRICT*/
    if ((tfil = (struct ap_prevstruct *) malloc (sizeof (*tfil))) ==
				(struct ap_prevstruct *) NULL)
	return (NOTOK);

    tfil -> ap_opeek = ap_peek;   /* save regular parse state info      */
    tfil -> ap_ogroup = ap_grplev;
    tfil -> ap_opersn = ap_perlev;
    tfil -> ap_prvgfunc = ap_gfunc;
    tfil -> ap_prvptr = ap_fle;   /* save previous stack entry          */
    ap_fle = tfil;                /* save current stack entry           */
    ap_iinit (gfunc);             /* create new parse state             */
    return (OK);
}

ap_ppop ()                        /* restore previous parse state       */
{
    register struct ap_prevstruct  *tfil;

    tfil = ap_fle;
    ap_peek = tfil -> ap_opeek;
    ap_grplev = tfil -> ap_ogroup;
    ap_perlev = tfil -> ap_opersn;
    ap_gfunc = tfil -> ap_prvgfunc;
    ap_fle = tfil -> ap_prvptr;
    free ((char *) tfil);
}
/**/

/*  the next three routines handle most of the overhead for acquiring
 *  the address list from a file.
 */

ap_flget ()                      /* get character from included file   */
{
    register int c;

    c = getc (ap_fle -> ap_curfp);

    if (c == '\n')
	return (',');           /* a minor convenience */

    return (c);
}

ap_fpush (file)                  /* indirect input from file           */
char   file[];
{
    if (ap_ppush (ap_flget) == NOTOK)   /* save current & set for file input */
	return (NOTOK);

    if ((ap_fle -> ap_curfp = fopen (file, "r")) == (FILE *) NULL)
    {                             /* couldn't get the file, tho         */
	ap_ppop ();
	return (NOTOK);
    }
    return (OK);
}

ap_fpop ()                       /* pop the stack, if any input nested */
{
    if (ap_fle -> ap_curfp != NULL)
	fclose (ap_fle -> ap_curfp);

    ap_ppop ();
}

/* ******************  PARSE LIST MANIPULATION  ********************* */

/*  these echo the basic list manipuation primitives, but use ap_pcur
 *  for the pointer and any insert will cause ap_pcur to be updated
 *  to point to the new node.
 */

ap_palloc ()                      /* alloc, insert after pcur           */
{
    ap_pnsrt (ap_alloc (), APP_ETC);
}

ap_pfill (obtype, obvalue)        /* add data to node at end of chain     */
char   obtype;
register char  *obvalue;
{
    extern char *strdup ();

    ap_pcur -> ap_obtype = obtype;
    ap_pcur -> ap_obvalue =
		(obvalue == (char *) 0) ? (char *) 0 : strdup (obvalue);
#if DEBUG > 1
    if (debug)
	printf ("(%s/'%s')", typtab[obtype], obvalue);
#endif
}

ap_pnsrt (ap, ptrtype)          /* add node to end of parse chain     */
register AP_ptr ap;
char    ptrtype;
{
    register AP_ptr rap_pcur;

    if ((rap_pcur = ap_pcur) -> ap_obtype == APV_NIL)
    {                             /* current one can be used            */
	rap_pcur -> ap_obtype = ap -> ap_obtype;
	rap_pcur -> ap_obvalue = ap -> ap_obvalue;
	ap -> ap_obvalue = 0;
	ap_free (ap);
    } else {                      /* really do the insert               */
	rap_pcur -> ap_ptrtype = ptrtype;
	rap_pcur -> ap_chain = ap;
	ap_pcur = ap;
    }
}

ap_pappend (obtype, obvalue)      /* alloc, fill, append at end         */
char    obtype;
char   *obvalue;
{
    ap_palloc ();                 /* will update pcur                   */
    ap_fllnode (ap_pcur, obtype, obvalue);
}

ap_padd (obtype, obvalue)         /* try to append data to current node */
char    obtype;
char  *obvalue;
{
    register AP_ptr nap;

    nap = ap_add (ap_pcur, obtype, obvalue);

    if (nap != OK)                /* created new node                   */
	ap_pcur = nap;
}
