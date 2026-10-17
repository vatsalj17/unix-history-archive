#
#include "../defines.h"
#include "../param.h"
#ifdef	DEBUG_SWAP
#include "../proc.h"
#include "../text.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"

struct map
	{
	unsigned m_size;
	unsigned m_addr;
	};
tightmap ()
{	register struct proc *p;
	register struct text *x;
	for (p=proc; p <= max_proc; p++)
		if (p->p_stat && p->p_stat != SZOMB && !(p->p_flag & SLOCK))
			if (p->p_flag & SLOAD )
			{     if (inmap(coremap,p->p_size,p->p_addr))
				printf ("proc / core map at proc[%d]\n", p - proc);
			} else {
			      if (inmap(swapmap,(p->p_size + 7) >> 3,p->p_addr))
				printf ("proc / swap map at proc[%d]\n", p - proc);
			}
	for (x=text; x < &text[NTEXT]; x++)
		if (x->x_iptr && x->x_ccount && x->x_caddr)
			if (inmap(coremap,x->x_size,x->x_caddr))
				printf ("text / core map at text[%d]\n", x - text);
}
/*
 *	this is a map consistency check for a very nasty bug
 */

inmap(mp, size, addr)
unsigned size, addr;
struct map	*mp;
{
	register struct map	*bp;

	/* find bp: bp[-1].m_addr <= addr < bp[0].m_addr */

	for(bp=mp; bp->m_size && bp->m_addr <= addr; bp++);

	if( bp > mp && (bp-1)->m_addr + (bp-1)->m_size > addr)
		return(1);
	if(bp->m_size && bp->m_addr < addr + size)
		return(1);
	return(0);
}
#endif	DEBUG_SWAP
