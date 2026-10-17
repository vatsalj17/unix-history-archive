#include "util.h"
#include "mmdf.h"
#include "ch.h"

/* cache some information */

extern	Llog	*logptr;
extern	char	*strdup();
extern	char	*malloc();

ca_add (cachep, hostid, value, ttl)	/* add entry to cache */
Cache	**cachep;
register char	*hostid;
int	value;
time_t	ttl;			/* time to live */
{
	register Cache *cap, *pcap;
	time_t	timenow;

#ifdef DEBUG
	ll_log (logptr, LLOGGEN, "cache add (%s %o %d)",
			hostid, value, ttl);
#endif
	cap = *cachep;
	pcap = cap;
	time(&timenow);
	for (; cap != NULL; ) {
		if (lexequ (cap->ca_hostid, hostid)) {
			/* Already in the list, reset values */
			cap->ca_value = value;
			cap->ca_expire = timenow + ttl;
#ifdef DEBUG
 			ll_log (logptr, LLOGPTR, "cache reset (%s, %s, %d)",
				hostid, rp_valstr (value), ttl);
#endif
			return (OK);
		} else if (cap->ca_expire <= timenow) {
			register Cache *tcap = cap;

			/* Look for deadwood, and expired record */
#ifdef DEBUG
 			ll_log (logptr, LLOGGEN, "cache clear (%s)",
				cap->ca_hostid);
#endif
			if (cap == pcap)
				*cachep = pcap = cap = cap->ca_next;
			else
				pcap->ca_next = cap = cap->ca_next;
			free (tcap->ca_hostid);
			free (tcap);
		} else {
			pcap = cap;
			cap = cap->ca_next;
		}
	}

	/* Not in the list, so we must add */
	/*NOSTRICT*/
	if ((cap = (Cache *)malloc(sizeof (struct cache))) == NULL)
		return (NOTOK);
	cap->ca_hostid = strdup (hostid);
	cap->ca_expire = timenow + ttl;
	cap->ca_value = value;
	cap->ca_next = *cachep;		/* Insert at head */
	*cachep = cap;
	return (OK);
}
/**/

ca_find (cachep, hostid)	/* is this host in the cache */
Cache	**cachep;
register char	*hostid;	/* key to data in cache */
{
	register Cache *cap, *pcap;
	time_t	timenow;

#ifdef DEBUG
	ll_log (logptr, LLOGBTR, "ca_find (%s)", hostid);
#endif

	cap = *cachep;
	pcap = cap;
	time(&timenow);
	for (; cap != NULL; ) {
		/* Look for deadwood */
		if (cap->ca_expire <= timenow) {
			register Cache *tcap = cap;

			/* Expired record */
#ifdef DEBUG
 			ll_log (logptr, LLOGGEN, "cache clear (%s)",
				cap->ca_hostid);
#endif
			if (cap == pcap)
				*cachep = pcap = cap = cap->ca_next;
			else
				pcap->ca_next = cap = cap->ca_next;
			free (tcap->ca_hostid);
			free (tcap);
		} else if (lexequ (cap->ca_hostid, hostid)) {
			/* Bingo! */
#ifdef DEBUG
 			ll_log (logptr, LLOGPTR, "cache hit (%s)", hostid);
#endif
			if (cap != pcap) {
				/* Move to the head of the class */
				pcap->ca_next = cap->ca_next;
				cap->ca_next = *cachep;
				*cachep = cap;
			}
			return (cap->ca_value);
		} else {
			pcap = cap;
			cap = cap->ca_next;
		}
	}
	return (0);
}
