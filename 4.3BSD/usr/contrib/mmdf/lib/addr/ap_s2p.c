#include "ap_lex.h"
#include "util.h"
#include "ap.h"

/*
 *		ap_s2p.c
 *
 *	One of the main uses of this routine is to replace adrparse.c
 */

LOCVAR char *s2p_txt;             /* hdr_in() passes to alst()      */

LOCFUN
	s2p_in ()               /* adrs extracted from component text */
{
    if (s2p_txt == (char *) 0)  /* nothing to give it                 */
	return (EOF);

    switch (*s2p_txt)
    {
	case '\0':
	case '\n':            /* end of string, DROP ON THROUGH     */
	    s2p_txt = (char *) 0;
	    return (0);
    }
    return (*(s2p_txt++));
}

char *
ap_s2p (strp, tree, group, name, local, domain, route)
char	*strp;
AP_ptr	*tree, *group, *name, *local, *domain, *route;
{
    s2p_txt = strp;
    *tree = ap_pinit (s2p_in);
    switch (ap_1adr ())
    {
    case DONE: 
    	return ((char *) DONE);
    case OK: 
    	ap_t2parts (*tree, group, name, local, domain, route);
	return (s2p_txt);	/* so they can parse the next chunk */
    }
    return ((char *) NOTOK);
}
