/*
 *              A L I A S F E T C H . C
 *
 *      This module hides the multiple file nature of the alias
 *	data.  Only submit should need to bypass this.
 */
#include "util.h"
#include "mmdf.h"
#include "ch.h"

extern Alias *al_list;

aliasfetch (first, key, dest, bypass)
int	first;
char	*key;
char	*dest;
int	bypass;
{
	register Alias *alp;
	register int badretval = NOTOK;
	register int ret;

	for (alp = al_list; alp != (Alias *)0; alp = alp->al_next) {
		if (bypass && !(alp->al_flags & AL_NOBYPASS))
			continue;
		if ((ret = tb_k2val (alp->al_table, first, key, dest)) == OK)
			return (OK);
		if (ret != NOTOK)
			badretval = ret;	/* NS timeout? */
	}
	return (badretval);
}
