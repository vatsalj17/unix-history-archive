#include "util.h"               /* from ../utildir */
#include "conf.h"               /* from ../mmdf/h */
#include "ch.h"
#include "ap.h"
#include "dm.h"
#include "ll_log.h"

/*  Format one address from pointers to constitutents, in a tree
 *
 *  Returns:    pointer to string if successful or
 *              NOTOK if error
 *
 *  SEK - using ap_p2s to output for ap_t2s has the general problem
 *              of losing comments.   Perhaps ap_t2s should be
 *              separate?
 */

extern LLog *logptr;
extern int ap_outtype;
extern char *multcat();
extern char *strdup();
extern char *ap_dmflip();
extern Domain *dm_v2route();

char *
ap_p2s (group, name, local, domain, route)
    AP_ptr  group,             /* beginning of group name  */
	    name,              /* beginning of person name  */
	    local,             /* beginning of local-part */
	    domain,            /* basic domain reference */
	    route;             /* beginning of 733 forward routing */
{
    Dmn_route   dmnroute;
    AP_ptr      lastptr;
    char        *routp;         /* 822 -> 733 route string */
    int         inperson,
		ingroup;
    register char *strp;        /* The string we are building */
    register char *cp;
    register AP_ptr curptr;
    char *flipptr;
    char *stripptr;
    char tmpbuf [LINESIZE];
    char *tmpdomain [DM_NFIELD];
    int tmpcnt;
    char buf[LINESIZE];         /* buf for dm_v2route                 */
    char *drefptr;              /* pointer to string to be output     */

#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "ap_p2s()");

    if ((ap_outtype & AP_822) == AP_822)   /* AP_733 is implicit default */
	ll_log (logptr, LLOGFTR, "AP_822 on");
    ll_log (logptr, LLOGFTR, (ap_outtype & AP_BIG)
	? "AP_BIG on" : "AP_LITTLE on");
    if ((ap_outtype & AP_NODOTS) == AP_NODOTS)  /* AP_DOTS is implicit def. */
	ll_log (logptr, LLOGFTR, "AP_NODOTS on");
#endif

    inperson = ingroup = FALSE;
    strp = strdup("");
    routp = strdup("");

    if (group != (AP_ptr) 0) {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "ap_p2s:  group is '%s'", group -> ap_obvalue);
#endif
	for (curptr = group; curptr != (AP_ptr)0; curptr = curptr -> ap_chain)
	{                             /* print munged addr                  */
	    switch (curptr -> ap_obtype) {
		default:
		case APV_NIL:
		    break;

		case APV_CMNT:        /* Output value as comment */
		    if (name != (AP_ptr) 0) {
				    /* only output comments for pretty forms */
			val2str (tmpbuf, curptr -> ap_obvalue, APV_CMNT);
			cp = multcat(strp, (strp[0]?" ":""), "(",tmpbuf,")", (char *)0);
			free (strp);
			if(cp == (char *)0)
			    return( (char *)NOTOK);
			strp = cp;
		    }
		    continue;

		case APV_NGRP:
		    ingroup = TRUE;
		case APV_GRUP:
		    val2str (tmpbuf, curptr -> ap_obvalue, APV_GRUP);
		    cp = multcat(strp, tmpbuf, (char *)0);
		    free (strp);
		    if(cp == (char *)0)
			return( (char *)NOTOK);
		    strp = cp;
		    continue;
	    }
	    break;
	}
	if (ingroup) {
	    cp = multcat(strp, ": ", (char *)0);
	    free (strp);
	    if(cp == (char *)0)
		return( (char *)NOTOK);
	    strp = cp;
	}
    }

    if (name != (AP_ptr) 0) {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "ap_p2s:  name is '%s'", name -> ap_obvalue);
#endif
	for (curptr = name; curptr != (AP_ptr)0; curptr = curptr -> ap_chain)
	{                         /* print munged addr                  */
	    switch (curptr -> ap_obtype) {
		default:
		case APV_NIL:
		    break;

		case APV_CMNT:        /* Output value as comment */
		    val2str (tmpbuf, curptr -> ap_obvalue, APV_CMNT);
		    cp = multcat(strp, (strp[0]?" " : ""), "(", tmpbuf, ")", (char *)0);
		    free (strp);
		    if(cp == (char *)0)
			return( (char *)NOTOK);
		    strp = cp;
		    continue;

		case APV_NPER:
		    inperson = TRUE;
		case APV_PRSN:
		    val2str (tmpbuf, curptr -> ap_obvalue, APV_PRSN);
		    cp = multcat(strp, tmpbuf, (char *)0);
		    free (strp);
		    if(cp == (char *)0)
			return( (char *)NOTOK);
		    strp = cp;
		    continue;
	    }
	    break;
	}
    }

    if (inperson) {
	cp = multcat(strp, " <", (char *)0);
	free (strp);
	if(cp == (char *)0)
	    return( (char *)NOTOK);
	strp = cp;
    }

    if (route != (AP_ptr) 0)      /* we have routing info */
    {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "ap_p2s:  route is '%s'", route -> ap_obvalue);
#endif
	for (lastptr = curptr = route; ; curptr = curptr -> ap_chain) {
	    if (curptr == (AP_ptr)0)    /* Grot Grot !!!!!!! */
		goto defcase1;
	    switch (curptr -> ap_obtype) {
		case APV_EPER:
		continue;

		defcase1:;	/* YEUCH !! */
		default:
		case APV_NIL:
		    if ((ap_outtype & AP_822) == AP_822) {
					/* piece of cake */
			cp = multcat(strp, ":", (char *)0);
			free (strp);
			if(cp == (char *)0)
			    return( (char *)NOTOK);
			strp = cp;
		    }
		    break;

		case APV_CMNT:        /* Output value as comment */
		    if (name != (AP_ptr) 0) {
			val2str (tmpbuf, curptr -> ap_obvalue, APV_CMNT);
			cp = multcat(strp, (strp[0]?" ":""), "(",tmpbuf,")", (char *)0);
			free (strp);
			if(cp == (char *)0)
			    return( (char *)NOTOK);
			strp = cp;
		    }
		    continue;

		case APV_DLIT:
		case APV_DOMN:
		    val2str (tmpbuf, curptr -> ap_obvalue, curptr -> ap_obtype);
		    flipptr = stripptr = (char *) 0;
		    drefptr = tmpbuf;
		    if (((ap_outtype & AP_BIG) == AP_BIG) ||
		       (((ap_outtype & AP_NODOTS) == AP_NODOTS) &&
				(curptr == lastptr)) ) {
		        /* check domain ref in either case */
			Domain  *lrval = dm_v2route (tmpbuf, buf, &dmnroute);

			if(lrval == (Domain *)MAYBE)
			    return( (char *)MAYBE);

			if (lrval != (Domain *) NOTOK) {
			    if ((ap_outtype & AP_NODOTS) == AP_NODOTS
				&& curptr == lastptr) {
				/* only strip domain on next-hop in route */
				tmpcnt = cstr2arg (dmnroute.dm_argv[0],
					DM_NFIELD, tmpdomain, '.');
				stripptr = strdup(tmpdomain[0]);
				drefptr = stripptr;
			    } 
			    else 
				if ((ap_outtype & AP_BIG) == AP_BIG) {
				    flipptr = ap_dmflip (buf);
				    drefptr = flipptr;
				}
			}
		    }

		    if ((ap_outtype & AP_822) == AP_822) {
					/* piece of cake */
		        cp = multcat(strp,(curptr!=lastptr?",@":"@"),drefptr,(char *)0);
			free (strp);
			if(cp == (char *)0)
			    return( (char *)NOTOK);
			strp = cp;
		    } else {
			if(routp[0] == '\0')
			    cp = multcat("@", drefptr, (char *)0);
			else
			    cp = multcat("%", drefptr, routp, (char *)0);
			free (routp);
			if(cp == (char *)0)
			    return( (char *)NOTOK);
			routp = cp;
		    }
		    if (flipptr != (char *)0)
			free (flipptr);
		    if (stripptr != (char *)0)
			free (stripptr);
		    continue;
	    }
	    break;
	}
    }

    if (local != (AP_ptr) 0) {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "ap_p2s:  local is '%s'", local -> ap_obvalue);
#endif
	for (curptr = local; curptr != (AP_ptr)0; curptr = curptr -> ap_chain) {
	    switch (curptr -> ap_obtype) {		/* print munged addr */
		default:
		case APV_NIL:
		    break;

		case APV_CMNT:        /* SEK - don't skip these */
		    if (name != (AP_ptr) 0) {
			val2str (tmpbuf, curptr -> ap_obvalue, APV_CMNT);
			cp = multcat(strp, (strp[0]?" ":""), "(",tmpbuf,")", (char *)0);
			free (strp);
			if(cp == (char *)0)
			    return( (char *)NOTOK);
			strp = cp;
		    }
		    continue;

		case APV_WORD:
		case APV_MBOX:
					/* SEK - YUK                    */
		    if (strindex (":Include:", curptr -> ap_obvalue) == 0)
			(void) strcpy (tmpbuf, curptr -> ap_obvalue);
		    else
			val2str (tmpbuf, curptr -> ap_obvalue, APV_MBOX);
		    cp = multcat(strp, tmpbuf, (char *)0);
		    free (strp);
		    if(cp == (char *)0)
			return( (char *)NOTOK);
		    strp = cp;
		    continue;
	    }
	    break;
	}
    }

    if (domain != (AP_ptr) 0) {
#ifdef DEBUG
	ll_log (logptr, LLOGFTR, "ap_p2s:  domain is '%s'",
		domain -> ap_obvalue);
#endif
	val2str (tmpbuf, domain -> ap_obvalue, domain -> ap_obtype);
	flipptr = stripptr = (char *) 0;
	drefptr = tmpbuf;
	if (((ap_outtype & AP_BIG) == AP_BIG) ||
	   (((ap_outtype & AP_NODOTS) == AP_NODOTS)) && route == (AP_ptr) 0) {
	    /* check domain ref in either case */
	    Domain  *lrval = dm_v2route (tmpbuf, buf, &dmnroute);

	    if(lrval == (Domain *)MAYBE)
		return( (char *)MAYBE);

	    if (lrval != (Domain *) NOTOK) {
		if (((ap_outtype & AP_NODOTS) == AP_NODOTS) &&
			(route == (AP_ptr) 0)) {
		    /* If there is no route, this domain is next hop: strip */
		    tmpcnt = cstr2arg (dmnroute.dm_argv[0], DM_NFIELD, 
					tmpdomain, '.');
		    stripptr = strdup(tmpdomain[0]);
		    drefptr = stripptr;
		}
		else
		    if ((ap_outtype & AP_BIG) == AP_BIG) {
			flipptr = ap_dmflip (buf);
			drefptr = flipptr;
		    }
	    }
	}

	if ((ap_outtype & AP_822) == AP_822 || routp[0] == '\0')  /* easy */
	    cp = multcat (strp, "@", drefptr, routp, (char *)0);
	else
	    cp = multcat (strp, "%", drefptr, routp, (char *)0);

	if(cp == (char *)0)
		return(cp);
	free (strp);
	strp = cp;
	if (flipptr != (char *) 0)
	    free (flipptr);
	if (stripptr != (char *) 0)
	    free (stripptr);
    }
    free (routp);

    if (inperson) {
	cp = multcat(strp, ">", (char *)0);
	free (strp);
	if(cp == (char *)0)
	    return( (char *)NOTOK);
	strp = cp;
    }
    if (ingroup) {
	cp = multcat(strp, ";", (char *)0);
	free (strp);
	if (cp == (char *)0)
    	    return( (char *)NOTOK);
	strp = cp;
    }
    return (strp);
}


/*
 *  This function is just barely usable.  The hole problem of
 *  quoted strings is hard to get right especially when the
 *  mail system is trying to make up for human forgetfulness.
 *                              -DPK-
 *  SEK - have improved this somewhat by giving knowledge of
 *  the various object types.  Does not handle strings of spaces.
 */
LOCFUN
	val2str (buf, value, obtype)      /* convert to canonical string */
    char *buf,
	 *value,
	 obtype;
{
    int gotspcl;
    int inquote;
    register char *fromptr,
		  *toptr;
#ifdef DEBUG
    ll_log (logptr, LLOGFTR, "val2str ('%s', %d)", value, obtype);
#endif


    if (obtype == APV_CMNT) {
	for (fromptr=value, toptr=buf; *fromptr != '\0'; *toptr++ = *fromptr++)
	    switch (*fromptr) {
		case '\r':
		case '\n':
		case '\\':
		case '(':
		case ')':
		    *toptr++ = '\\';
	    }
	*toptr = '\0';
	return;
    }

    if (obtype == APV_DLIT) {
	for (fromptr=value, toptr=buf; *fromptr != '\0'; *toptr++ = *fromptr++)
	    switch (*fromptr) {
		case '\r':
		case '\n':
		case '\\':
		    *toptr++ = '\\';
		    continue;
		case '[':
		    if (fromptr != value)
		       *toptr++ = '\\';
		    continue;
		case ']':
		    if (*(fromptr + 1) != (char) 0)
			*toptr++ = '\\';
	    }
	*toptr = '\0';
	return;
    }

    inquote = FALSE;
    for (gotspcl = FALSE, fromptr = value; *fromptr != '\0'; fromptr++) {
	switch (*fromptr) {
	    case '"':
		inquote = (inquote == TRUE ? FALSE : TRUE);     /* Flip-Flop */
		break;

	    case '\\':
	    case '\r':
	    case '\n':
		if (inquote == FALSE) {
		    gotspcl = TRUE;
		    goto copyit;
		}
		break;

	    case '<':
	    case '>':
	    case '@':
	    case ',':
	    case ';':
	    case ':':
	    case '\t':
	    case '[':
	    case ']':
	    case '(':
	    case ')':
		if( inquote == FALSE) {
		    gotspcl = TRUE;
		    goto copyit;
		}
		break;

	    case ' ':
		if (inquote == FALSE)
		    if ((obtype == APV_DOMN) || (obtype == APV_MBOX)) {
			gotspcl = TRUE;
			goto copyit;
		    } else {
				/* SEK hack to handle " at "              */
				/* yes - this really is needed          */
			if ((uptolow(*(fromptr + 1)) == 'a') &&
			    (uptolow(*(fromptr + 2)) == 't') &&
			    (*(fromptr + 3) == ' ')) {
			    gotspcl = TRUE;
			    goto copyit;
			}
		    }
		break;

	    case '.':
		if (inquote == FALSE && ((obtype == APV_GRUP) || (obtype == APV_PRSN)))
		{
		    gotspcl = TRUE;
		    goto copyit;
		}
		break;
	}
    }

    gotspcl = inquote;          /* If were in a quote, something's wrong */
copyit:
    toptr = buf;
    if (gotspcl)
	*toptr++ = '"';
    for (fromptr = value; *fromptr != '\0'; *toptr++ = *fromptr++)
	switch (*fromptr) {
	    case '\r':
	    case '\n':
	    case '\\':
	    case '"':
		if (gotspcl)
		    *toptr++ = '\\';
	}
    if (gotspcl)
	*toptr++ = '"';
    *toptr = '\0';
}
