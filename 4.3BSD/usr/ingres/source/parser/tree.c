# include	<stdio.h>
# include	<ingres.h>
# include	<aux.h>
# include	<tree.h>
# include	<pv.h>
# include	"parser.h"
# include	<symbol.h>
# include	<sccs.h>
# include	<errors.h>

SCCSID(@(#)tree.c	8.3	2/8/85)


/*
** TREE
**  	FUNCTION TO ADD NODE TO QUERY TREE
**	RETURN VALUE IS POINTER TO NODE JUST CREATED		
*/
QTREE *
tree(lptr, rptr, typ, len, valu, attnum)
QTREE			*lptr;
QTREE			*rptr;
char			typ;
int			len;
register int		valu;
register struct atstash	*attnum;
{
	register QTREE	*tptr;
	extern char	Trfrmt;
	extern char	Trfrml;
	extern char	*need();
	extern QTREE	*norm();
	extern int	Err_current;

# ifdef	xPTR3
	tTfp(55, 0, "tree type(%d), len(%d), value(%d).\n", typ, len, valu);
# endif

	if (Err_current)
		return (NULL);

	/* Following is a hack.  Sorry about that John.  */
	if (typ == AND)
		len = sizeof (struct rootnode) - sizeof (short);
	
	tptr = (QTREE *) need(Qbuf, QT_HDR_SIZ + len);
	tptr->left = lptr;
	tptr->right = rptr;
	tptr->sym.type = typ;
	tptr->sym.len = len;

	switch (typ)
	{
	  case VAR:
		tptr->sym.value.sym_var.varno = valu & I1MASK;
		tptr->sym.value.sym_var.attno = attnum->atbid;
		tptr->sym.value.sym_var.varfrmt = attnum->atbfrmt;
		tptr->sym.value.sym_var.varfrml = attnum->atbfrml;
		tptr->sym.value.sym_var.valptr = NULL;
		tptr->sym.value.sym_var.varstr = NULL;
		break;

	  case ROOT:
	  case AGHEAD:
		tptr->sym.value.sym_root.rootuser = valu;
		break;

	  case TREE:
	  case BYHEAD:
	  case AND:
	  case OR:
	  case QLEND:
		break;

	  case UOP:
	  case BOP:
		tptr->sym.value.sym_op.opno = valu;
		format(tptr);
		break;

	  case COP:
		if ((tptr->sym.value.sym_op.opno = getcop(valu)) == BADCOP)
		{
			/* bad const operator */
			par_error(BADCONSTOP, WARN, valu, 0);
			return(NULL);
		}
		break;

	  case AOP:
		format(tptr->right);
		tptr->sym.value.sym_op.agfrmt = Trfrmt;
		tptr->sym.value.sym_op.agfrml = Trfrml;

	  case RESDOM:
		tptr->sym.value.sym_resdom.resno = valu;
		format(tptr);
		tptr->sym.value.sym_resdom.resfrmt = Trfrmt;
		tptr->sym.value.sym_resdom.resfrml = Trfrml;
		break;

	  default:
		/* INT, FLOAT, CHAR */
		bmove(valu, &tptr->sym.value, len & I1MASK);
		break;
	}
	return (tptr);
}

/*
** WINDUP
**	assign resno's to resdoms of an agg fcn
*/
windup(ptr)
QTREE	*ptr;
{
	register int			tot;
	register int			kk;
	register QTREE			*t;

	/* COUNT THE RESDOM'S OF THIS TARGET LIST */
	kk = 1;
	for (t = ptr; t; t = t->left)
		kk++;
	tot = 1;
	for (t=ptr; t;t = t->left)
		t->sym.value.sym_resdom.resno = kk - tot++;
}

/*
** ADDRESDOM - makes a new entry for the target list
**
**	Trname must contain the name of the resdom to
**	use for the header, create and Rsdmno for append, replace
**
**	the parameters are pointers to the subtrees to be
**	suspended from the node
*/
QTREE *
addresdom(lptr, rptr)
QTREE	*lptr, *rptr;
{
	register QTREE	*rtval;
	register struct atstash		*aptr;
	char				buf[10];	/* buffer type and length in ascii for dbu */

	extern int			Opflag;
	extern int			Rsdmno;
	extern int			Equel;
	extern int			Resrng;
	extern char			Trfrmt;
	extern char			Trfrml;
	extern char			*Trname;
	extern PARRNG			Parrng[];

	extern QTREE			*tree();
	extern struct atstash		*attlookup();

	int				temp;

	switch (Opflag)
	{
	  case mdSTOP:
		rtval = NULL;
		break;
	  case mdRETR:
	  case mdRET_UNI:
	  case mdVIEW:
		Rsdmno++;
		if (Rsdmno >= MAXDOM)
			/* too many resdoms */
			par_error(RESXTRA, FATAL, 0);
		rtval = tree(lptr, rptr, RESDOM, sizeof (struct resdomnode), Rsdmno);
		if (!Equel || Resrng)
		{
			/* buffer info for header or CREATE */
			setp(PV_STR, Trname);

			buf[0] = Trfrmt & I1MASK;
			smove(iocv(Trfrml & I1MASK), &buf[1]);

			setp(PV_STR, buf);
		}
		break;

	  default:
		/*
		** for append and replace, the result domain
		** number is determined by the location of
		** the attribute in the result relation
		*/
		if (sequal(Trname, "tid"))
			/* attrib not found */
			par_error(NOATTRIN, WARN, Trname,
			    trim_relname(Parrng[Resrng].vardesc.reldum.relid), 0);
#		ifdef	DISTRIB
		if (sequal(Trname, "sid"))
			/* attrib not found */
			par_error(NOATTRIN, WARN, Trname,
			    trim_relname(Parrng[Resrng].vardesc.reldum.relid), 0);
#		endif
		aptr = attlookup(Resrng, Trname);
		Rsdmno = aptr->atbid;
		rtval = tree(lptr, rptr, RESDOM, sizeof (struct resdomnode), Rsdmno);
		if (Opflag != mdPROT)	/* INTEGRITY not possible here */
			attcheck(aptr);
		break;
	}
	return (rtval);
}
/*
** GETCOP
**	routine to lookup 'string' in constant operators table
**	constant table is declared in tables.y
**	structure is defined in ../parser.h
*/
getcop(string)
char	*string;
{
	register struct constop	*cpt;
	register char		*sptr;
	extern struct constop	Coptab[];

	sptr = string;
	for (cpt = Coptab; cpt->copname; cpt++)
		if (sequal(sptr, cpt->copname))
			return (cpt->copnum);
	return (BADCOP);
}

/*
** SUBSTRING
**	creates structure to save delimiters of a substring
**	structure is defined in ../h/tree.h
*/
STRKEEPER
*substring(str,isname)
char	*str;
int     isname;
{
	extern char	*need();
	STRKEEPER	*s;

	s = (STRKEEPER *) need(Qbuf,sizeof(STRKEEPER));
	s->number[0] = 1;
	s->string[0] = str;
	if (isname)
	    s->flag[0] = 1;
	if (str == NULL)
	    s->flag[0] |= 2;
	return(s);
}

STRKEEPER
*endvals(interval,left,right)
STRKEEPER *interval;
int	  left,right;
{
	if (left == '(')
		interval->type[0] = OPEN;
	else
		interval->type[0] = CLOSED;
	if (right == ')')
		interval->type[1] = OPEN;
	else
		interval->type[1] = CLOSED;
	return(interval);
}

setnumber(interval,num)
STRKEEPER *interval;
int	  *num;
{
	interval->number[0] = *num;
}


groupstrings(left,right)
STRKEEPER *left,*right;
{
	left->string[1] = right->string[0];
	left->flag[1] = right->flag[0];
	left->number[1] = right->number[0];
}


/*
**	CHECK_BNF -- check the legality of a simplified BNF defnition
**
**		Parameters: 
**			str-- the string to be checked
**	
**		Returns:
**			0 - the string is legal
**			<0 - the string is not legal
**				-1 : bracket,brace not matched
**				-2 : hyphen misused
**
**		Called by:
**			make_tuples
**
**		Comments: 
**			the string may not contain nested braces or brackets
**			these chars have special meaning and must be 
**			backslashed: { } [ ] - \
**
*/

check_bnf(str)
char	*str;
{
	char	*temp;		/* temp ptr to string */
	int	len;		/* length of string */
	char	ch;		/* ptr to one char of string */
	char 	nextch;
	int	inbrace=0;	/* keeps track of braces */
	int 	inbrak=0;	/* keeps track of brackets */


	len = strlen(str);
	temp = str;

	while (len > 0)
	{
		len--;
		ch = *temp++;

		switch (ch)
		{
		    case LBRACKET:
			if (!inbrace)
				inbrak++;
			else
				return(-1);
			break;
		    case RBRACKET:
			inbrak--;
			if (inbrak != 0)
				return(-1);
			break;
		    case LBRACE:
			if (!inbrak)
				inbrace++;
			else
				return(-1);
			break;
		    case RBRACE:
			inbrace--;
			if (inbrace != 0)
				return(-1);
			break;
		    case '-':
			return(-2);
			break;
		    case '\\':
			*temp++;
			break;
		    default:
			nextch = *temp;
			if (nextch == '-')
			{
				*temp++;
				len--;
				if (!len)
					return(-2);
				ch = *temp;
				switch(ch)
				{
				    case LBRACKET:
				    case RBRACKET:
				    case LBRACE:
				    case RBRACE:
				    case '-':
					return(-2);
					break;
				    case '\\':
					*temp++;
					break;
				    default:
					break;
				}
			}
		}
	}
	if ((inbrace) || (inbrak))
		return(-1);
	return(0);
}


/*
**	MAKE_TUPLES -- create the tuples for the 'rdelim' relation
**			as specified by a user-defined delimitor
**
**		Paramaters:
**			desc--descriptor for the relation
**			group--group name for the delimitor
**			delim--name of the delimitor
**			str-bnf string specifying the delimitor
**
**		Returns:
**			0 if successful
**			<0 if not successful
**			-1,-2: BNF expression not legal
**
*/
make_tuples(desc,group,delim,str)
DESC 	*desc;
char	*group;
char	*delim;
char	*str;
{
	int	err;		/* error status of bnf string */
	char	*map;		/* pointer to next string to make into bitmap */
	int	len;		/* len of str */
	int 	mlen;		/* len of substring to make into bitmap */
	int 	order;		/* order of bitmap */
	int 	type;		/* type of interval ONE or ZEROMORE */
	char	ch;		/* pointer to current char */

	err = check_bnf(str);
	if (err < 0)
		return(err);

	len = strlen(str);
	order = 0;

	while (len > 0)
	{
		order++;
		map = str;
		mlen = 0;

		ch = *str++;
		len--;

		switch (ch)
		{
			case LBRACKET:
			    type = ONE;
			    map = str;
			    while ((ch = *str++) != RBRACKET)
			    {
				mlen++;
				len--;
				if (ch == '\\')
				{
					ch = *str++;
					mlen++;
					len--;
				}
			    }
			    len--;
			    break;
			
			case LBRACE:
			    type = ZEROMORE;
			    map = str;
			    while ((ch = *str++) != RBRACE)
			    {
				mlen++;
				len--;
				if (ch == '\\')
				{
					ch = *str++;
					mlen++;
					len--;
				}
			    }
			    len--;
			    break;

			default:
			    type = ONE;
			    if (ch == '\\')
			    {
				map = str;
				ch = *str++;
				len--;
				mlen = 1;
			    }
			    if (*str == '-')
			    {
				*str++;
				len--;
				mlen++;
				*str++;
				len--;
				mlen++;
			    }
			    else
				mlen = 1;
			    break;
		}

		create_tup(desc,order,group,delim,type,map,mlen);
	}
	return(0);
}



/*
**	CREATE_TUP-- create a tuple in the 'rdelim' relation
**
**		Parameters:
**			desc - descriptor for the relation
**			order - order field for tuple
**			group - group field for tuple
**			delim - delim field for tuple
**			type - type field for tuple
**			str - string to be converted into bitmap
**			strlen - length of str
**
**		Called by:
**			make_tuples
*/
create_tup(desc,order,group,delim,type,str,strlen)
DESC	*desc;
int	order;
char	*group;
char	*delim;
int	type;
char	*str;
int	strlen;
{
	DELIM_TUP	*tuple;
	char		bitmap[BITMAPLEN];
	TID		*tid;
	char		*malloc();
	char		*make_dmap();
	char		b[BITMAPLEN];
	int		i;


	tuple = (DELIM_TUP *) malloc (sizeof(DELIM_TUP));
	tuple->order = order;
	strcpy(tuple->group,group);
	strcpy(tuple->delim,delim);
	tuple->type = type;

	make_dmap(str,strlen,b);
	for ( i= 0; i< BITMAPLEN; i++)
		tuple->bitmap[i] = b[i];

	insert(desc,&tid,tuple,1);
}


/*
**	MAKE_DMAP -- given a BNF string, make the corresponding bitmap
**
**		Parameters:
**			str - BNF string
**		 	len - length of string
**		
**		Called by:
**			create_tup
**
**		Returns:
**			pointer to the bitmap of 16 chars
**
**		Comments:
**			The bitmap is formed of 16 chars. The total bits
**		(128) represents the characters of the ASCII set.
**		If the BNF string indicates a character, the bit
**		corresponding to that char is set in the bitmap.
**		All other bits are reset.
*/
char *
make_dmap(str,len,b)
char	*str;
int	len;
char	*b;
{
	char	ch;
	char	nextch;
	int	i;

# ifdef xPTR3
	tTfp(42,0,"DMAP: str = %s, len = %d\n",str,len);
# endif
	for (i = 0; i < ACHARS; i++)
		reset(b,i);

	while (len > 0)
	{
		ch = *str++;
		len--;
		if (ch == '\\')
		{
			ch = *str++;
			len--;
		}
		if ( (len > 0) && (*str == '-'))
		{
			*str++;
			len--;
			nextch = *str++;
			len--;
			for (i = ch; i <= nextch; i++)
			{
				set(b,i);
			}
		}
		else
		{	
			set(b,ch);
		}
	}
	return(b);
}

/*
**	SET,RESET -- bitmap setting routines
**
**		Parameters:
**			map: the array of chars which forms the bitmap
**			n: the bit to set or reset
**	
**		Called by:
**			make_bitmap
**
*/
set(map,n)
char 	*map;
int	n;
{
	map[n/BITS] |= (1<<(n%BITS));
}

reset(map,n)
char	*map;
int	n;
{
	map[n/BITS] &= ((1<<(n%BITS)) ^ MAXFIELD); 
}

test(map,n)
char	*map;
int	n;
{
	return ((map[n/BITS] & (1<<(n%BITS))) != 0);
}

/*
**	MAKE_LIST -- puts the delimitors to be used in the delim queue
**
**		Parameters:
**			desc - descriptor for the relation
**			group - group of delims to use
**
**		Returns:
**			0 if ok
**			-1 if no delims could be found in the specified group
**
**		Comments:
**			given a group name, adds all delimitors in that
**			group to the head of the delim queue, which is
**			pointed to by Delimhead.
**			if the queue is empty, the predefined delimitors
**			'w' and 'c' will be added to the list
*/
make_list(desc,group)
DESC 	*desc;
char	*group;
{
	DELIM_TUP		tuple;
	TID			lotid, hitid;
	extern DELIMLIST	*Delimhead;
	DELIMLIST		*d;
	DMAP			*map, *m;
	char			delim[12];
	int			start = 1;
	extern char		*malloc();
	int			i;
	int			notfound = 1;

# ifdef xPTR3
	tTfp(42,0,"Make_list: group = %s\n", group);
# endif
	if (!strcmp (group,"system"))
	{
		predef_delims();
		return(0);
	}
	if (find(desc,LRANGEKEY, &lotid, &hitid, group) < 0)
		return(-1);
	find(desc,HRANGEKEY, &lotid, &hitid, group);
	while (!get(desc, &lotid, &hitid, &tuple, 1))
	{
		if (strcmp(tuple.group, group))
		{
			continue;
		}
		notfound = FALSE;
		/* check if it is a new delimitor */
		if (strcmp(tuple.delim, delim))
			start = 1;

		/* start a new delimitor node */
		if (start)
		{
			d = (DELIMLIST *) malloc( sizeof(DELIMLIST)); 
			strcpy(delim, tuple.delim);
			strcpy(d->group,tuple.group);
			strcpy(d->delim,delim);
			d->back = Delimhead;
			Delimhead = d;

			map = (DMAP *) malloc(sizeof(DMAP));
			map->order = tuple.order;
			map->type = tuple.type;
			for ( i = 0; i < BITMAPLEN; i++)
				map->bits[i] = tuple.bitmap[i];
			map->next = NULL;
			d->maptr = map;
			m = map;
			start = 0;

		}
		else 	/* add another bitmap to the delimitor node */
		{
			map = (DMAP *) malloc(sizeof(DMAP));
			map->order = tuple.order;
			map->type = tuple.type;
			for ( i = 0; i < BITMAPLEN; i++)
				map->bits[i] = tuple.bitmap[i];
			map->next = NULL;
			m->next = map;
			m = m->next;

		}

	}
	/*prlist(Delimhead); */
	if (notfound)
		return(-1);
	return(0);
}

/*
**	PREDEF_DELIMS - add the predefined delims to the queue
**
**		Called by:
**			make_list
**
**		Side Effects:
**			the delim queue pointed to by Delimhead
**			is initialized with the delims 'w' and 'c'.
**
*/
predef_delims()
{
	DELIMLIST	*d;
	DMAP		*m, *m2;
	int		i;

	d = (DELIMLIST * ) malloc(sizeof(DELIMLIST));
	strcpy(d->group, "system");
	strcpy(d->delim, "c");
	d->back = NULL;

	m = (DMAP *) malloc(sizeof (DMAP));
	m->order = 1;
	m->type = 0;
	for (i = ' '; i <= '~'; i++)
		set(m->bits, i);
	m->next = NULL;
	d->maptr = m;
	Delimhead = d;


	d = (DELIMLIST * ) malloc(sizeof(DELIMLIST));
	strcpy(d->group, "system");
	strcpy(d->delim, "w");
	d->back = NULL;
	m = (DMAP *) malloc(sizeof (DMAP));
	m->order = 1;
	m->type = 0;
	for (i = 'A'; i <= 'Z'; i++)
		set(m->bits, i);
	for (i = 'a'; i <= 'z'; i++)
		set(m->bits, i);
	d->maptr = m;

	m2 = (DMAP *) malloc(sizeof(DMAP));
	m2->order = 2;
	m2->type = 1;
	for (i = 'A'; i <= 'Z'; i++)
		set(m2->bits, i);
	for (i = 'a'; i <= 'z'; i++)
		set(m2->bits, i);
	m->next = m2;
	m2->next = NULL;

	d->back = Delimhead;
	Delimhead = d;
}





/*
**	PRLIST -- print contents of delimiter queue
*/
prlist(d)
struct delimlist	*d;
{
	struct delimlist	*q;
	DMAP			*m;
	int			i;

	printf("DELIM QUEUE:\n");
	q = d;
	while (q != NULL)
	{
		printf("-------------------------------------------------------\n");
		printf("NODE:   group= %s,  delim = %s \n", q->group, q->delim);
		m = q->maptr;
		while (m != NULL)
		{
			printf("maps:\n");
			printf("order = %d, type = %d \n", m->order, m->type);
			for (i = 0; i < ACHARS; i++)
				printf("%d ", test(m->bits,i));
			printf("\n");
			m = m->next;
		}
		q = q->back;
		printf("-------------------------------------------------------\n");
	}

		return(0);

}

/*
**	SHRINK_LIST -- remove the delims in specified group from list
**
**		Parameters:
**			group - name of the group to remove
**
*/
shrink_list(group)
char	*group;
{
	struct delimlist		*p, *q;
	extern struct delimlist		*Delimhead;

	/* may not delete sytem delims */
	if (!strcmp(group, "system"))
		return(-1);

	p = Delimhead;

	while ((p != NULL) && (strcmp(p->group, group)))
	{
		q = p;
		p = p->back;
	}
	if (p == NULL)
	{
		return(-1);	/* error group not found */
	}

	while(!strcmp(p->group, group))
	{
		if (p == Delimhead)
			Delimhead = p->back;
		else
			q->back = p->back;
		
		if (p->back ==  NULL)
		{
			return(0);
		}

		p = p-> back;
	}

	/* prlist(Delimhead); */
	return(0);
}

/*
**	DESTROY_DELIM -- remove the group of delims from the relation 'rdelim'
**
**		Parameters:
**			group - the group of delims to remove
**
**		Called By:
**			grammar.y
**	
**		Returns:
**			0 if delims were successfully removed
**			-1 if delims were not found in relation
*/
destroy_delim(desc, group)
DESC	*desc;
char	*group;
{
	DELIM_TUP	tuple;
	TID		lotid,hitid;
	int		notfound = 1;

	if (find(desc,LRANGEKEY, &lotid, &hitid, group) < 0)
		return(-1);
	find(desc,HRANGEKEY, &lotid, &hitid, group);

	while (!get(desc, &lotid, &hitid, &tuple, 1))
	{
		if (!strcmp(tuple.group, group))
		{
			notfound = 0;
			delete(desc,&lotid);
		}
	}
	if (notfound) 
		return(-1);
	return(0);
}
