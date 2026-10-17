/*
 *      TB_NS.C
 *
 * Original version by Phil Cockcroft to use MF and MD records.
 * Also had several speed improvements such as caching.  Later
 * reworked by Phil to do MX lookups.
 *
 * Mostly rewritten by Craig Partridge to move routing decisions
 * into SMTP channel.
 *
 */

#include "util.h"
#include "mmdf.h"
#include "ch.h"
#include "ns.h"
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/resolv.h>

extern char *strncpy();

/*
 * if you want caching, define NSCACHE
 */

#define NSCACHE 25

/*
 * next definition should go away as all servers use MX
 * right now if they don't we just try the address.  MF and MD are
 * dead!
 */

#define OLDSERVER

#ifndef MAXADDR
#define MAXADDR		10
#endif

#ifndef MAXADDR_PER
#define MAXADDR_PER	2
#endif

#ifndef MAXDATA
#define MAXDATA (4 * PACKETSZ)		/* tcp tried after udp */
#endif

#ifndef MAXMX
#define MAXMX	(MAXADDR)		/* shouldn't be < MAXADDR */
#endif

extern  struct  ll_struct *logptr;
extern  char    *strdup(), *strcpy();
extern  char *locmachine, *locdomain, *locname;

union ansbuf {			/* potentially huge */
    HEADER ab1;
    char ab2[MAXDATA];
}; 

union querybuf {		/* just for outbound stuff */
    HEADER qb1;			/* didn't want to clobber stack */
    char qb2[2 * MAXDNAME];
}; 

LOCVAR  struct in_addr mx_addrs[MAXADDR];	/* cache of MX addrs */
LOCVAR  int max_mxa= -1, on_mxa= -1;	/* indicies into cache of MX addrs */
LOCVAR  char dn_name[MAXDNAME];
LOCVAR  char local[MAXDNAME];

#ifdef NSCACHE
struct ns_cache {
    int nc_type;
    int nc_count;
    int nc_time;
    char *nc_key;
    char *nc_data;
};

LOCVAR  struct ns_cache *cachedata[NSCACHE+1];
LOCVAR  int cachetime;
#endif

/*
 * table fetch routine for ns
 */

ns_fetch(table, key, value, first)
Table   *table;          /* What "table" are we searching */
char    *key;
char    *value;         /* Where to put result */
int     first;          /* now used */
{
    register char *tmp;
    char lbuf[LINESIZE];
    int	type;
    int rval;

#ifdef  DEBUG
    ll_log(logptr, LLOGFTR, "ns_fetch (%o, %s, %d)",
	table->tb_flags, key, first);
    ll_log(logptr, LLOGFTR, "ns_fetch: timeout (%d), rep (%d), servers (%d)",
	_res.retrans, _res.retry, _res.nscount);
#endif

    type = (table->tb_flags & TB_TYPE);

    if (first)
    {
	switch(type)
	{

	    case TB_DOMAIN:     /* name -> official domain name */
		sprintf(lbuf, "%s.%s", key, table->tb_name);
		key = lbuf;
		break;

	    case TB_ROOT:      /* name -> official domain name (special case) */
	    case TB_CHANNEL:     /* official domain name -> list of addresses */
		break;

	    default:
#ifdef  DEBUG
		ll_log(logptr, LLOGTMP, "unknown table type '%o'",type);
#endif
		return(NOTOK);
	} /* end switch */

	max_mxa = on_mxa = -1;

	if (!cachehit(key,(type==TB_CHANNEL?T_MX:T_CNAME),&rval))
	{
	    if (type == TB_CHANNEL)
	    {
		if ((rval = ns_getmx(key, &max_mxa, mx_addrs, MAXADDR)) == OK)
		{
		    on_mxa = 0;
#ifdef NSCACHE
		    cacheaddr(key,T_MX,max_mxa,mx_addrs);
#endif
		}
	    }	
	    else
	    {
		rval = ns_getcn(key, dn_name, sizeof(dn_name));
#ifdef NSCACHE
		if (rval == OK)
		    cachename(key,T_CNAME,dn_name);
		else if (rval == MAYBE)
		    cachename(key,0,(char *)0);
#endif
	    }
	}

	if (rval != OK)
	{
#ifdef DEBUG
	    if (rval == MAYBE)
		ll_log(logptr, LLOGFTR, "nameserver query timed out");
	    else
		ll_log(logptr, LLOGFTR, "nameserver query failed");
#endif
	    return(rval);
	}
    }

    /* O.K. now give answer */

    switch (type)
    {
	case TB_CHANNEL:
	    /* if NS failure we returned MAYBE above */
	    if ((max_mxa <= 0) || (on_mxa >= max_mxa))
		return(NOTOK);

	    tmp = (char *)&(mx_addrs[on_mxa++]);

	    (void) sprintf(value,"%u.%u.%u.%u",
		    ((unsigned)tmp[0]) & 0xff, ((unsigned)tmp[1]) & 0xff,
		    ((unsigned)tmp[2]) & 0xff, ((unsigned)tmp[3]) & 0xff);
	    break;

	default:
	    /* can't get multiple names */
	    if (!first)
		return(NOTOK);

	    /* give them the name */
	    (void) strcpy(value,dn_name);
	    break;
    }
    
#ifdef  DEBUG
    ll_log(logptr, LLOGFTR, "NS returns '%s'", value);
#endif
    return(OK);
}

/*
 * see if name is not cannonical name
 */

LOCFUN
ns_getcn(key, name, namelen)
char *key, *name;
int namelen;
{
    register int n;
    register char *cp;
    union querybuf qbuf;
    union ansbuf abuf;
    HEADER *hp;
    char *eom;
    extern char *ns_skiphdr();

#ifdef DEBUG
    ll_log(logptr, LLOGFTR, "ns_getcn(%s)",key);
#endif

    n = res_mkquery(QUERY, key, C_IN, T_CNAME, (char *)0, 0, (char *)0,
	(char *)&qbuf, sizeof(qbuf));

    /* what else can we do? */
    if (n < 0)
	return(NOTOK);

    n = res_send((char *)&qbuf,n,(char *)&abuf, sizeof(abuf));

    if (n < 0)
    {
#ifdef DEBUG
	ll_log(logptr, LLOGFTR, "ns_getcn: bad return from res_send");
#endif
	return(MAYBE);
    }

    hp = (HEADER *)&abuf;

    if (hp->rcode != NOERROR)
	return(ns_error(hp));

    if (ntohs(hp->ancount) == 0)
    {
	/* it is the official name */
	(void) strncpy(name,key,namelen);
#ifdef DEBUG
	ll_log(logptr, LLOGFTR, "ns_getcn: %s -> %s",key,name);
#endif
	return(OK);
    }

    /* only get here on NOERRR with ancount != 0 */
#ifdef DEBUG
    ll_log(logptr, LLOGFTR, "ns_getcn: parsing answer to query");
#endif

    /* skip header */
    eom = ((char *)&abuf)+n;

    if ((cp = ns_skiphdr((char *)&abuf, hp, eom)) == 0)
	return(MAYBE);

    /* one CNAME is enough */
    if ((n = dn_expand((char *)&abuf,eom, cp, name, namelen))<0)
	return(MAYBE);

    /* skip to name */
    cp += (n + (3 * sizeof(u_short)) + sizeof(u_long));

    if ((n = dn_expand((char *)&abuf, eom, cp, name, namelen))<0)
	return(MAYBE);
    
#ifdef DEBUG
    ll_log(logptr, LLOGFTR, "ns_getcn: %s -> %s",key,name);
#endif
    return(OK);
}

/*
 * build a list of addresses of MX hosts to try....
 */

LOCFUN
ns_getmx(key, max, mxtab, tsize)
char *key;
int *max;
struct in_addr mxtab[];
int tsize;
{
    register char *cp;
    register int i, j, n;
    HEADER *hp;
    struct hostent *he;
    union querybuf qbuf;
    union ansbuf abuf;
    u_short type, dsize;
    int pref, localpref;
    int count, mxcount;
    int sawmx;			/* are we actually processing mx's? */
    char *eom;
    char buf[MAXDNAME];		/* for expanding in dn_expand */
    char newkey[MAXDNAME]; 	/* in case we get a CNAME RR back... */
    struct {			/* intermediate table */
	char *mxname;
	u_short mxpref;
    } mx_list[MAXMX];
    extern char *ns_skiphdr();

#ifdef DEBUG
    ll_log(logptr, LLOGFTR, "ns_getmx(%s, %x, %x, %d)",key,max,mxtab,tsize);
#endif

restart:

    sawmx = 0;

    n = res_mkquery(QUERY, key, C_IN, T_MX, (char *)0, 0, (char *)0,
	(char *)&qbuf, sizeof(qbuf));

    /* what else can we do? */
    if (n < 0)
	return(NOTOK);

#ifdef DEBUG
    ll_log(logptr, LLOGFTR, "ns_getmx: sending ns query (%d bytes)",n);
#endif

    n = res_send((char *)&qbuf,n,(char *)&abuf, sizeof(abuf));

    if (n < 0)
    {
#ifdef DEBUG
	ll_log(logptr, LLOGFTR, "ns_getmx: bad return from res_send");
#endif
	return(MAYBE);
    }

    hp = (HEADER *)&abuf;

#ifdef OLDSERVER
    if ((hp->rcode != NOERROR) && (hp->rcode != FORMERR))
#else
    if (hp->rcode != NOERROR) 
#endif /* OLDSERVER */
	return(ns_error(hp));

#ifdef OLDSERVER
    if ((ntohs(hp->ancount) == 0) || (hp->rcode == FORMERR))
#else
    if (ntohs(hp->ancount) == 0)
#endif /* OLDSERVER */
    {
	mxcount = 1;
	mx_list[0].mxname = strdup(key);
	mx_list[0].mxpref = 0;
	goto doaddr;
    }

    /* read MX list */
    sawmx = 1;
    count = ntohs(hp->ancount);

    /* need local machine name */
    if (local[0] == '\0')
    {
	if ((locmachine != 0) && (*locmachine != '\0'))
	    (void) sprintf(local,"%s.%s.%s",locmachine,locname,locdomain);
	else
	    (void) sprintf(local,"%s.%s",locname,locdomain);
    }

    /* skip header */
    eom = ((char *)&abuf) + n;
    if ((cp = ns_skiphdr((char *)&abuf, hp, eom))==0)
	return(MAYBE);

    /* get them MX's */
    localpref = -1;
    mxcount = 0;
    *max = 0;

#ifdef DEBUG
    ll_log(logptr, LLOGFTR, "ns_getmx: %d answers to query",count);
#endif

    while ((cp < eom) && (count--))
    {
	n = dn_expand((char *)&abuf,eom, cp, buf, sizeof(buf));
	if (n < 0)
	    goto quit;

	cp += n;
	type = getshort(cp);
	/* get to datasize */
	cp += (2 * sizeof(u_short)) + sizeof(u_long);
	dsize = getshort(cp);
	cp += sizeof(u_short);

	/*
	 * is it an MX ? 
	 * note it could be a CNAME if we didn't use a domain lookup
	 */

	if (type == T_CNAME)
	{
	    ll_log(logptr,LLOGTMP,"ns_getmx: CNAME answer to MX query");
	    n = dn_expand((char *)&abuf,eom, cp, newkey, sizeof(newkey));
	    cp += dsize;
	    if (n < 0)
		continue;	/* pray? */
#ifdef DEBUG
	    ll_log(logptr,LLOGFTR,"ns_getmx: %s -> %s (new query)",key,newkey);
#endif DEBUG
	    key = newkey;
	    goto restart;
	}

	if (type != T_MX)
	{
	    ll_log(logptr,LLOGTMP,"ns_getmx: RR of type %d in response",type);
	    cp += dsize;
	    continue;       /* keep trying */
	}

	pref = getshort(cp);
	cp += sizeof(u_short);

	n = dn_expand((char *)&abuf,eom, cp, buf, sizeof(buf));
	if (n < 0)
	    goto quit;

	cp += n;

	/* is it local? */
	if ((lexequ(local,buf)) && ((localpref < 0) || (pref < localpref)))
	{
	    localpref = pref;
	    for(i=(mxcount-1); i >= 0; i--)
	    {
		if (mx_list[i].mxpref < localpref)
		    break;

		(void) free(mx_list[i].mxname);
		mxcount--;
	    }
	    continue;
	}

	/* now, see if we keep it */
	if ((localpref >= 0) && (pref >= localpref))
	    continue;

	/*  find where it belongs */
	for(i=0; i < mxcount; i++)
	    if (mx_list[i].mxpref > pref)
		break;

	/* not of interest */
	if (i == MAXMX)
	    continue;

	/* shift stuff to make space */
	for(j=mxcount-1; j > i; j--)
	{
	    if (j==(MAXMX-1))
		(void) free(mx_list[j].mxname);

	    mx_list[j].mxname = mx_list[j-1].mxname;
	    mx_list[j].mxpref = mx_list[j-1].mxpref;
	}

	mx_list[i].mxname = strdup(buf);
	mx_list[i].mxpref = pref;

	if (mxcount <= i)
	    mxcount = i + 1;
    }

    /*
     * should read additional RR section for addresses and cache them
     * but let's hold on that.
     */

doaddr:
    /* now build the address list */
#ifdef DEBUG
    ll_log(logptr, LLOGFTR,"ns_getmx: using %d mx hosts",mxcount);
#endif

    for(i=0,j=0; (i < mxcount) && (j < tsize); i++)
    {
	/*
	 * note that gethostbyname() is slow -- we should cache so
	 * we don't ask for an address repeatedly
	 */

	he = gethostbyname(mx_list[i].mxname);

	if (he == 0)
	{
#ifdef DEBUG
	    ll_log(logptr, LLOGFTR, "ns_getmx: no addresses for %s",
		mx_list[i].mxname);
#endif
	    /* nope -- were trying special case and no address */
	    if ((!sawmx) && (h_errno != TRY_AGAIN))
		return(NOTOK);

	    continue;
	}

	for(n=0; (j < tsize) && (n < MAXADDR_PER); n++, j++)
	{
	    if (he->h_addr_list[n] == 0)
		break;

	    bcopy(he->h_addr_list[n],(char *)&mxtab[j],sizeof(struct in_addr));
	}
#ifdef DEBUG
	ll_log(logptr, LLOGFTR, "ns_getmx: %d addresses saved for %s",
		n, mx_list[i].mxname);
#endif
    }
    *max = j;

quit:
    for(i=0; i < mxcount; i++)
	(void) free(mx_list[i].mxname);

#ifdef DEBUG
    if (*max == 0)
	ll_log(logptr, LLOGFTR, "ns_getmx: problems parsing response to query");
#endif

    return (*max == 0 ? MAYBE : OK);
}

/*
 * figure out proper error code to return given an error
 */

LOCFUN
ns_error(hp)
register HEADER *hp;
{

#ifdef DEBUG
    ll_log(logptr, LLOGFTR, "ns_error: server returned code %d",hp->rcode);
#endif

    switch (hp->rcode)
    {
	case NXDOMAIN:
	    return(NOTOK); /* even if not authoritative */

	case SERVFAIL:
	    return(MAYBE);

	default:
	    break;
    }

    return(NOTOK);
}

/*
 * skip header of query and return pointer to first answer RR.
 */

LOCFUN
char *ns_skiphdr(answer, hp, eom)
char *answer;
HEADER *hp;
register char *eom;
{
    register int qdcount;
    register char *cp;
    register int n;
    char tmp[MAXDNAME];

    qdcount = ntohs(hp->qdcount);

    cp = answer + sizeof(HEADER);

    while ((qdcount-- > 0) && (cp < eom))
    {
	n = dn_expand(answer,eom,cp,tmp,sizeof(tmp));
	if (n < 0)
	    return(0);
	cp += (n + QFIXEDSZ);
    }

    return((cp < eom)? cp : 0);
}

/*
 * routine to set the resolver timeouts
 * takes maximum number of seconds you are willing to wait
 */

ns_settimeo(ns_time)
int     ns_time;
{
    register int i;
    register int retry, retrans;
    static int called = 0;
    static struct state oldres;

    if ((_res.options & RES_INIT) == 0)
	    res_init ();

    /* always start afresh */
    if (called)
    {
	bcopy((char *)&oldres,(char *)&_res,sizeof(oldres));
    }
    else
    {
	called = 1;
	bcopy((char *)&_res,(char *)&oldres,sizeof(oldres));
    }
    

    /* too small? */
    if (ns_time <= (NS_MINTIME))
    {
	retry = NS_MINRETRY;
	retrans = NS_MINRETRANS;
	goto done;
    }

    retry = _res.retry;
    retrans = _res.retrans;

    /* bigger than bind? */
    if (ns_time > (retry * retrans))
    {
	/* can't increase servers, and no point to retrans so up retry a bit */
	while ((retry < NS_MAXRETRY) && (ns_time > ((retry+1)*retrans)))
	    retry++;

	goto done;
    }

    /* reduce timer periods */

    for(i=0; ns_time < (retry * retrans); i++)
    {
	switch (i%2)
	{
	    case 0:
		/* cut down rexmits first */
		if (retry > NS_MINRETRY)
		{
		    retry--;
		    continue;
		}
		/* fall thru */

	    case 1:
		/* then interval between rexmits */
		if (retrans > NS_MINRETRANS)
		{
		    retrans /= 2;
		    if (retrans < NS_MINRETRANS)
			retrans = NS_MINRETRANS;
		}
		continue;
	}
    }

done:

    _res.retrans = retrans;
    _res.retry = retry;

#ifdef DEBUG
    ll_log(logptr, LLOGFTR, "ns_timeo: servers(%d), retrans(%d), retry(%d)",
	_res.nscount, _res.retrans, _res.retry);
#endif
}

/*
 * Caching stuff starts here....
 * cache stores following pairs:
 *        key -> mx address list
 *        key -> canonical name
 *        key -> we timed out on
 *
 * must store complete answers to a table fetch  -- storing by RR's
 * leads to incomplete information in the cache -- and thus busted
 * routing.
 */

/*
 * see if key/type pair is in cache.  If so, set rval appropriately
 */

cachehit(key,type,rval)
char *key;
int type;
int *rval;
{
#ifdef NSCACHE
    register int i;
    register struct ns_cache *cp;
    register struct in_addr *ip1, *ip2;

    *rval = OK;

    for(i=0; i < NSCACHE; i++)
    {
	cp = cachedata[i];

	if (cp->nc_type && (cp->nc_type != type))
	    continue;

	if (!lexequ(key,cp->nc_key))
	    continue;

	/* got a hit */
	cp->nc_time = cachetime++;
#ifdef DEBUG
	ll_log(logptr,LLOGFTR,"ns: cache hit, type %d",cp->nc_type);
#endif /* DEBUG */

	switch (cp->nc_type)
	{
	    case 0:	/* key we timed out on */
		*rval = MAYBE;
		return(1);

	    case T_MX:
		/* fill mx cache */
		max_mxa = cp->nc_count;
		on_mxa = 0;

		ip1 = (struct in_addr *)cp->nc_data;
		ip2 = mx_addrs;
		for(i=0; i < max_mxa; i++)
		    *ip2++ = *ip1++;

#ifdef DEBUG
		ll_log(logptr,LLOGFTR,"ns: found %d addresses for %s in cache",
		    max_mxa, key);
#endif
		
		return(1);

	    case T_CNAME:
		/* fill in dn_name */
		if (cp->nc_count)
		    (void) strcpy(dn_name,cp->nc_data);
		else
		    (void) strcpy(dn_name,cp->nc_key);
		return(1);
	    }
    }

#endif /* NSCACHE */
    /* no one should look at rval, but... */
    *rval = NOTOK;
    return(0);
}

#ifdef NSCACHE

cacheput(cp)
struct ns_cache *cp;
{
    register int i;
    register int d;

    cp->nc_time = cachetime++;

#ifdef DEBUG
    ll_log(logptr,LLOGFTR,"ns: cache put (key=%s, type=%d, count=%d)",
	    cp->nc_key, cp->nc_type, cp->nc_count);
#endif

    for(d=0,i=0; i < NSCACHE; i++)
    {
	if (cachedata[i] == 0)
	{
	    cachedata[i] = cp;
	    return;
	}

	if (cachedata[i]->nc_time < cachedata[d]->nc_time)
	    d = i;
    }

#ifdef DEBUG
    ll_log(logptr,LLOGFTR,"ns: full cache, discarding (%s,%d)", cp->nc_key,
	    cp->nc_type);
#endif

    /* must delete someone */
    (void) free(cachedata[d]->nc_key);
    if (cachedata[d]->nc_count)
	(void) free(cachedata[d]->nc_data);
    (void) free((char *)cachedata[d]);

    /* put new one in */
    cachedata[d] = cp;
}

cacheaddr(key,type,count,list)
char *key;
int type, count;
struct in_addr *list;
{
    register struct ns_cache *cp;
    register struct in_addr *ip1, *ip2;
    extern char *malloc(), *calloc();

    /* NOSTRICT */
    if ((cp = (struct ns_cache *)malloc(sizeof(*cp)))==0)
	return;

    /* NOSTRICT */
    if ((cp->nc_data = calloc((unsigned)count,sizeof(*list)))==0)
    {
	(void) free((char *)cp);
	return;
    }
    if ((cp->nc_key = strdup(key))==0)
    {
	(void) free(cp->nc_data);
	(void) free((char *)cp);
	return;
    }

    cp->nc_count = count;
    cp->nc_type = type;

    ip1 = list;
    ip2 = (struct in_addr *)cp->nc_data;

    while (count--)
	*ip2++ = *ip1++;

    cacheput(cp);
}

cachename(key,type,name)
char *key, *name;
int type;
{
    register struct ns_cache *cp;
    extern char *malloc(), *strdup();

    /* NOSTRICT */
    if ((cp = (struct ns_cache *)malloc(sizeof(*cp)))==0)
	return;

    if ((cp->nc_key = strdup(key))==0)
    {
	(void) free((char *)cp);
	return;
    }

    if ((name != 0) && !lexequ(key,name))
    {
	if ((cp->nc_data = strdup(key)) == 0)
	{
	    (void) free(cp->nc_key);
	    (void) free((char *)cp);
	    return;
	}
	cp->nc_count = 1;
    }
    else
    {
	cp->nc_data = 0;
	cp->nc_count = 0;
    }

    cp->nc_type = type;

    cacheput(cp);
}
#endif NSCACHE
