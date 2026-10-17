/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */
#include	"defs.h"
#include	"cnodes.h"
#include	"tokens.h"
#include	"io.h"

/*
 *	Memory allocation routines.
 */

/*
 *	ALLOC_SLICE	- how many pages to sbrk at a time
 *	ALLOC_SIZE	- allocation page size (power of 2)
 *	ALLOC_MASK	- mask to round to page size
 *	ALLOC_FRAG	- size of largest chunk that we will
 *			  ignore on a chunk split
 *	STR_INC		- quantum of OUTZs worth of string page pointers
 */
#define	ALLOC_SLICE	16
#define	ALLOC_SIZE	512
#define	ALLOC_MASK	511
#define	ALLOC_FRAG	64
#define	STR_INC		16
#define	CFRAG_GRABZ	((ALLOC_SIZE - sizeof (phead)) / sizeof (cfrag))
#define	CNODE_GRABZ	((ALLOC_SIZE - sizeof (phead)) / sizeof (cnode))
#define	TOKEN_GRABZ	((ALLOC_SIZE - sizeof (mlist)) / sizeof (token))
#define	XNODE_GRABZ	((ALLOC_SIZE - sizeof (phead)) / sizeof (xnode))

typedef struct mlist	mlist;
typedef struct phead	phead;

/*
 *	Memory chunks are headed with an mlist structure which
 *	tells the size (including the header).  When free they
 *	are linked.
 */
struct mlist
{
	long	ml_size;
	mlist	*ml_next;
};

/*
 *	Pages which represent data local to a function are headed
 *	by phead structs which are linked together.
 */
struct phead
{
	phead	*ph_next;
};

extern char	*brk();
extern char	*sbrk();

char	*alloc_ptr;
char	*alloc_end;
mlist	*mem_list;
phead	*free_pages;
phead	*used_pages;

cfrag	*kfree;
cfrag	*ktmp;
cnode	*cnfree;
cnode	*ctmp;
token	*tfree;
token	*ttmp;
xnode	*xfree;
xnode	*xtmp;

/*
 *	Utility routines.
 */

/*
 *	Block copy memory.
 */
static void
bcopy(from, to, n)
register char	*from;
register char	*to;
register long	n;
{
	while (--n >= 0)
		*to++ = *from++;
}

/*
 *	Report free store exhaustion.
 */
static void
ran_out()
{
	fatal("ran out of memory");
}

/*
 *	Raw allocation routines.
 */

/*
 *	Make the first chunk from the memory up to the end of the next page.
 */
void
init_alloc()
{
	register long	sz;

	alloc_ptr = sbrk(0);
	sz = 2 * ALLOC_SIZE - ((long)alloc_ptr & ALLOC_MASK);

	if ((int)sbrk((int)sz) == SYSERROR)
		ran_out();

	alloc_end = alloc_ptr + sz;

}

/*
 *	Allocate some memory (at least a page).
 */
static char	*
allocate(sz)
register long	sz;
{
	register long	get;
	register char	*ret;
	static long	left;
	static char	*ptr;

	if (ptr == NULL)
		ptr = sbrk(0);

	if (sz < ALLOC_SIZE)
		internal("allocate", "alloc < ALLOC_SIZE");

	/*
	 *	If not enough get some more (at least SLICE pages).
	 */
	if (left < sz)
	{
		get = sz - left;

		if (get < ALLOC_SLICE * ALLOC_SIZE)
			get = ALLOC_SLICE * ALLOC_SIZE;

		if ((int)sbrk((int)get) == SYSERROR)
			ran_out();

		left += get;
	}

	ret = ptr;
	ptr += sz;
	left -= sz;

	return ret;
}

/*
 *	Chunk (heap) routines.
 */

/*
 *	Allocate a chunk from the free list or of fresh memory.
 *	This memory may be later freed with free_chunk.
 */
char	*
chunk(want)
int	want;
{
	register mlist	**n;
	register mlist	*m;
	register long	get;
	register long	sz;
	register long	remains;

	sz = want + sizeof (mlist);

	/*
	 *	Check the free list (which will typically be empty).
	 *	Split a block if more than FRAG remains.
	 */
	for (n = &mem_list; *n != NULL; n = &(*n)->ml_next)
	{
		if ((*n)->ml_size >= sz)
		{
			remains = (*n)->ml_size - sz;

			if (remains < ALLOC_FRAG)
			{
				m = *n;
				*n = (*n)->ml_next;
				return (char *)m + sizeof (mlist);
			}
			else
			{
				(*n)->ml_size = remains;
				((mlist *)((char *)*n + remains))->ml_size = sz;
				return (char *)*n + remains + sizeof (mlist);
			}
		}
	}

	/*
	 *	Use allocate to get some fresh memory.
	 *	Perhaps save the remainder.
	 */
	if (sz < ALLOC_SIZE)
		get = ALLOC_SIZE;
	else
		get = sz;

	m = (mlist *)allocate(get);
	remains = get - sz;

	if (remains < ALLOC_FRAG)
	{
		m->ml_size = get;
		return (char *)m + sizeof (mlist);
	}
	else
	{
		m->ml_size = remains;
		m->ml_next = mem_list;
		mem_list = m;
		((mlist *)((char *)m + remains))->ml_size = sz;
		return (char *)m + remains + sizeof (mlist);
	}
}

/*
 *	Free a chunk.
 */
void
free_chunk(p)
char	*p;
{
	register mlist	*m;

	m = (mlist *)(p - sizeof (mlist));
	m->ml_next = mem_list;
	mem_list = m;
}

/*
 *	Change the size of a chunk (like realloc).
 */
char	*
rechunk(p, want)
char	*p;
int	want;
{
	register mlist	*m;
	register char	*q;
	register long	sz;

	sz = want + sizeof (mlist);
	m = (mlist *)(p - sizeof (mlist));

	if (sz <= m->ml_size)
		return p;

	q = chunk((int)sz);
	bcopy((char *)m, q, m->ml_size);
	free_chunk(p);
	((mlist *)q)->ml_size = sz;
	return q + sizeof (mlist);
}

/*
 *	Manage extensible buffers.
 */
char	*
alloc_vector(p, n)
char	*p;
int	n;
{
	if (p == NULL)
		return chunk(n);
	else
		return rechunk(p, n);
}

/*
 *	alloc() routines (permanent allocation).
 */

/*
 *	Refill the alloc buffer.
 */
char	*
alloc_fill(want)
int	want;
{
	register long	sz;
	register mlist	*m;
	register mlist	**n;

	sz = want + sizeof (mlist);

	/*
	 *	Search the free list.
	 */
	for (n = &mem_list; *n != NULL; n = &(*n)->ml_next)
	{
		if ((*n)->ml_size >= sz)
		{
			m = *n;
			*n = (*n)->ml_next;
			alloc_end = (char *)m + m->ml_size;
			alloc_ptr = (char *)m + sz;
			return (char *)m + sizeof (mlist);
		}
	}

	/*
	 *	Use fresh memory.
	 */
	if (want < ALLOC_SIZE)
		sz = ALLOC_SIZE;
	else
		sz = want;

	alloc_ptr = allocate(sz);
	alloc_end = alloc_ptr + sz;
	alloc_ptr += want;
	return alloc_ptr - want;
}

/*
 *	Character string allocation routines.
 */

int		cstr_hiwater;
static int	cstr_first;
static mlist	*cstr_mlist;
static phead	*cstr_plist;
static char	*cstr_end;
static char	*cstr_ptr;

/*
 *	Record the fact that we have allocated character string space.
 *	Initialisation routines will release when hiwater is set.
 */
static void
cstr_new()
{
	if (cstr_first)
		cstr_hiwater = 1;
	else
		cstr_first = 1;
}

/*
 *	Release character string memory.
 */
void
cstr_release()
{
	{
		register mlist	*p;
		register mlist	*q;

		for (p = cstr_mlist; p != NULL; p = q)
		{
			q = p->ml_next;
			p->ml_next = mem_list;
			mem_list = p;
		}
	}

	{
		register phead	*p;
		register phead	*q;

		for (p = cstr_plist; p != NULL; p = q)
		{
			q = p->ph_next;
			p->ph_next = free_pages;
			free_pages = p;
		}
	}

	cstr_hiwater = 0;
	cstr_first = 0;
	cstr_mlist = NULL;
	cstr_plist = NULL;
	cstr_end = NULL;
	cstr_ptr = NULL;
}

/*
 *	Allocate a character string.
 */
char	*
cstr_alloc(s, n)
register char	*s;
register int	n;
{
	register char	*p;

	if (n > ALLOC_SIZE - sizeof (mlist))
	{
		register mlist	*m;

		/*
		 *	If a string bigger than a page it
		 *	is allocated from the heap.
		 */
		cstr_new();
		p = chunk(n);
		m = (mlist *)(p - sizeof (mlist));
		m->ml_next = cstr_mlist;
		cstr_mlist = m;
	}
	else
	{
		if (cstr_ptr + n > cstr_end)
		{
			register phead	*h;
			/*
			 *	Allocate a new page.
			 */

			cstr_new();
			h = (phead *)allocate((long)ALLOC_SIZE);
			h->ph_next = cstr_plist;
			cstr_plist = h;
			cstr_ptr = (char *)h + sizeof (phead);
			cstr_end = (char *)h + ALLOC_SIZE;
		}

		p = cstr_ptr;
		cstr_ptr += n;
	}

	/*
	 *	Copy string.
	 */
	{
		register char	*q;

		q = p;

		while (--n >= 0)
			*q++ = *s++;
	}

	return p;
}

/*
 *	Per function allocation routines.
 */

/*
 *	Release the per-function memory.
 */
void
release_memory()
{
	register phead	*p;
	register phead	*q;

	for (p = used_pages; p != NULL; p = q)
	{
		q = p->ph_next;
		p->ph_next = free_pages;
		free_pages = p;
	}

	used_pages = NULL;
	cnfree = NULL;
	kfree = NULL;
	xfree = NULL;

	cstr_release();
}

/*
 *	Get a new page either of the free list or fresh memory.
 */
static char	*
new_page()
{
	register phead	*p;

	if (free_pages != NULL)
	{
		p = free_pages;
		free_pages = p->ph_next;
	}
	else
		p = (phead *)allocate((long)ALLOC_SIZE);

	p->ph_next = used_pages;
	used_pages = p;

	return (char *)p + sizeof (phead);
}

cnode	*
creplenish()
{
	register int	i;
	register cnode	*x;
	register cnode	*y;

	y = (cnode *)new_page();

	for (i = 0, x = NULL; i < CNODE_GRABZ; i++)
	{
		y->c_next = x;
		x = y++;
	}

	return x;
}

cfrag	*
kreplenish()
{
	register int	i;
	register cfrag	*x;
	register cfrag	*y;

	y = (cfrag *)new_page();

	for (i = 0, x = NULL; i < CFRAG_GRABZ; i++)
	{
		y->k_next = x;
		x = y++;
	}

	return x;
}

token	*
treplenish()
{
	register int	i;
	register token	*x;
	register token	*y;

	y = (token *)chunk(TOKEN_GRABZ * sizeof (token));

	for (i = 0, x = NULL; i < TOKEN_GRABZ; i++)
	{
		y->t_next = x;
		x = y++;
	}

	return x;
}

xnode	*
xreplenish()
{
	register int	i;
	register xnode	*x;
	register xnode	*y;

	y = (xnode *)new_page();

	for (i = 0, x = NULL; i < XNODE_GRABZ; i++)
	{
		y->x_left = x;
		x = y++;
	}

	return x;
}

/*
 *	String (identifier name) table management routines.
 */

static char	*str_ptr;
static char	*str_end;
static char	**str_pages;
static int	str_count;
static int	str_limit;
static long	str_index;

/*
 *	Initialise string routines.
 */
void
init_str()
{
	str_index = 1;
}

/*
 *	Dump string tables to object file.
 */
long
dump_strings()
{
	register int	i;
	register int	j;
	extern int	out_fid;

	if (out_fid != NO_FID)
	{
		for (i = 0, j = str_count - 1; i < j; i++)
		{
			if (Fwrite(out_fid, str_pages[i], (long)OUTZ) == SYSERROR)
				fatal("write error");
		}

		if (str_ptr != str_end && 
		    Fwrite(out_fid, str_pages[i], 
		      (long)(str_ptr - str_pages[i])) == SYSERROR)
			fatal("write error");
	}

	return str_index - 1;
}

/*
 *	Allocate a string and return its index.
 */
char	*
str_alloc(s, len, p)
register char	*s;
register int	len;
long		*p;
{
	register char	*q;
	register char	*r;

	if (str_index == 0)
	{
		/*
		 *	Keywords are being installed.
		 */
		*p = 0;
		return s;
	}

	*p = str_index;
	str_index += len;
	q = str_ptr;

	/*
	 *	We want to use the string table storage space
	 *	for the print name.  If the string straddles a
	 *	page we can't do this so we must alloc() it.
	 */
	if (q + len > str_end)
	{
		register char	*e;
		register char	*t;

		r = alloc(len);
		t = r;
		e = str_end;

		while (--len >= 0)
		{
			if (q == e)
			{
				q = allocate((long)OUTZ);

				if (str_count == str_limit)
				{
					str_limit += STR_INC;
					str_pages = vector(str_pages, str_limit, char *);
				}

				e = &q[OUTZ];
				str_pages[str_count++] = q;
			}

			*q++ = *s;
			*t++ = *s++;
		}

		str_end = e;
		str_ptr = q;
	}
	else
	{
		r = q;

		while (--len >= 0)
			*q++ = *s++;

		str_ptr = q;
	}

	return r;
}
