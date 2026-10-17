#include "util.h"
#include "mmdf.h"

/*
 *     MULTI-CHANNEL MEMO DISTRIBUTION FACILITY  (MMDF)
 *
 *
 *     Copyright (C) 1979,1980,1981  University of Delaware
 *
 *     Department of Electrical Engineering
 *     University of Delaware
 *     Newark, Delaware  19711
 *
 *     Phone:  (302) 738-1163
 *
 *
 *     This program module was developed as part of the University
 *     of Delaware's Multi-Channel Memo Distribution Facility (MMDF).
 *
 *     Acquisition, use, and distribution of this module and its listings
 *     are subject restricted to the terms of a license agreement.
 *     Documents describing systems using this module must cite its source.
 *
 *     The above statements must be retained with all copies of this
 *     program and may not be removed without the consent of the
 *     University of Delaware.
 *
 *
 *     version  -1    David H. Crocker    March   1979
 *     version   0    David H. Crocker    April   1980
 *     version  v7    David H. Crocker    May     1981
 *     version   1    David H. Crocker    October 1981
 *
 */
/*                  String Labels for Reply Values                      */

char   *
	rp_valstr (val)           /* return text string for reply value */
    int    val;
{
    static char noval[] = "*** Illegal:  0000";
				  /* (noval[0] == '*') => illegal       */

    switch (rp_gval (val))
    {
	case RP_DONE:
	    return ("DONE");

	case RP_OK:
	    return ("OK");

	case RP_MOK:
	    return ("MOK");

	case RP_HOK:
	    return ("HOK");

	case RP_DOK:
	    return ("DOK");

	case RP_MAST:
	    return ("MAST");

	case RP_SLAV:
	    return ("SLAV");

	case RP_AOK:
	    return ("AOK");

	case RP_NET:
	    return ("NET");

	case RP_BHST:
	    return ("BHST");

	case RP_DHST:
	    return ("DHST");

	case RP_LIO:
	    return ("LIO");

	case RP_NIO:
	    return ("NIO");

	case RP_LOCK:
	    return ("LOCK");

	case RP_EOF:
	    return ("EOF");

	case RP_NS:
	    return ("NS");

	case RP_AGN:
	    return ("AGN");

	case RP_TIME:
	    return ("TIME");

	case RP_NOOP:
	    return ("NOOP");

	case RP_FIO:
	    return ("FIO");

	case RP_FCRT:
	    return ("FCRT");

	case RP_PROT:
	    return ("PROT");

	case RP_RPLY:
	    return ("RPLY");

	case RP_MECH:
	    return ("MECH");

	case RP_NO:
	    return ("NO");

	case RP_NDEL:
	    return ("NDEL");

	case RP_HUH:
	    return ("HUH");

	case RP_NCMD:
	    return ("NCMD");

	case RP_PARM:
	    return ("PARM");

	case RP_UCMD:
	    return ("UCMD");

	case RP_USER:
	    return ("USER");

	case RP_FOPN:
	    return ("FOPN");

	case RP_NAUTH:
	    return ("NAUTH");

	default:                  /* print illegal octal value          */
	    noval[15] = rp_gbbit (val) + '0';
	    noval[16] = rp_gcbit (val) + '0';
	    noval[17] = rp_gsbit (val) + '0';
	    return (noval);
    }
}
