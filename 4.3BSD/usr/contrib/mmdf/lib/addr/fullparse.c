#include "ap_lex.h"
#include "util.h"
#include "ap.h"

/*  Build full parse tree from an address list                            */

/*  I expect this not to be used very often, but it was convenient to
    provide and will work just fine when there are small address
    lists.

    It repeatedly invokes ap_1adr to acquire address segments, linking
    them together and NXT's.

*/

/*  < 1978  B. Borden       Wrote initial version of parser code
 *  78-80   D. Crocker      Reworked parser into current form
 */
extern AP_ptr ap_pcur,    /* Current ap node ptr              */
	      ap_pstrt;   /* Beginning of parse tree          */

#if DEBUG > 1
extern char debug;		  /* True if in debug mode                */
extern char *statnam[],
           *ptrtab[],
           *typtab[];
#endif

/**/

AP_ptr
ap_fullparse (gchfunc)
char    (*gchfunc) ();
{
	AP_ptr	ap_fp;        /* Beginning of list                    */
	char	more;
	register AP_ptr ap_sp, ap_prevp = NULL;
	int	naddrs = 0;

	ap_fp = ap_pinit (gchfunc);
	more = 0;
	for (;;)
	{
		ap_sp = ap_pcur;
		switch (ap_1adr ()) {
		case NOTOK: 
			return ((AP_ptr)NOTOK);
		case DONE: 
			if (--more > 0)   /* > 0 means files are stacked          */
			    ap_fpop (); /* just drop on through                 */
			else if (naddrs)
			    return(ap_fp);
			else
			    return ((AP_ptr)DONE);
		case OK: 
			naddrs++;
			switch (ap_sp -> ap_obtype) {
			case APV_DTYP: 
			    if (lexequ ("Include", ap_sp -> ap_obvalue))
			    {	  /* indirect through a file              */
#if DEBUG > 1
				if (debug)
					printf ("(File:%s)",
					    ap_sp -> ap_chain -> ap_obvalue);
#endif
				if (ap_fpush (ap_sp -> ap_chain -> ap_obvalue)
				    != OK)
					return ((AP_ptr)NOTOK);
				more++;
				if ((ap_pcur = ap_sqdelete (ap_sp, ap_pcur)) == 0)
					ap_palloc ();
				else
					ap_pnsrt (ap_alloc (), APP_NXT);
				if (ap_fp == ap_sp)
					ap_fp = ap_pcur;
				if (ap_prevp != ap_sp)
					ap_prevp -> ap_chain = ap_pcur;
				continue;
			    }
			default:
#if DEBUG > 1
			    printf ("[Got an address]\n", ap_pcur);
#endif
			    ap_pcur -> ap_ptrtype = APP_NXT;
			    ap_prevp = ap_pcur;
			    ap_pstrt = ap_pcur = ap_alloc();
			    ap_prevp -> ap_chain = ap_pstrt;
			    continue;
			}
		}
	}
}
