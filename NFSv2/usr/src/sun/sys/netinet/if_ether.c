#ifndef lint
static	char sccsid[] = "@(#)if_ether.c 1.1 85/05/30 Copyr 1983 Sun Micro";
#endif

/*
 * Copyright (c) 1983 by Sun Microsystems, Inc.
 *
 * from UCB 6.3 83/12/15
 */

/*
 * Ethernet address resolution protocol.
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/socket.h"
#include "../h/time.h"
#include "../h/kernel.h"
#include "../h/errno.h"
#include "../h/ioctl.h"

#include "../net/if.h"
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/ip.h"
#include "../netinet/if_ether.h"

#define	ARPTAB_BSIZ	6		/* bucket size */
#define	ARPTAB_NB	16		/* number of buckets */
#define	ARPTAB_SIZE	(ARPTAB_BSIZ * ARPTAB_NB)
struct	arptab arptab[ARPTAB_SIZE];
int	arptab_size = ARPTAB_SIZE;	/* for arp command */

#define	ARPTAB_HASH(a) \
	(((u_long)a) % ARPTAB_NB)

#define	ARPTAB_LOOK(at,addr) { \
	register n; \
	at = &arptab[ARPTAB_HASH(addr) * ARPTAB_BSIZ]; \
	for (n = 0 ; n < ARPTAB_BSIZ ; n++,at++) \
		if (at->at_iaddr.s_addr == addr) \
			break; \
	if (n >= ARPTAB_BSIZ) \
		at = 0; }

int	arpt_age;		/* aging timer */

/* timer values */
#define	ARPT_AGE	(60*1)	/* aging timer, 1 min. */
#define	ARPT_KILLC	20	/* kill completed entry in 20 mins. */
#define	ARPT_KILLI	3	/* kill incomplete entry in 3 minutes */

struct ether_addr etherbroadcastaddr = {{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }};
extern struct ifnet loif;

#define	IPENADDR 1		/* enable passing enaddr's thru IP layer */

/*
 * Timeout routine.  Age arp_tab entries once a minute.
 */
arptimer()
{
	register struct arpcom *ac;
	register struct arptab *at;
	register i;

	timeout(arptimer, (caddr_t)0, hz);
	if (++arpt_age > ARPT_AGE) {
		arpt_age = 0;
		at = &arptab[0];
		for (i = 0; i < ARPTAB_SIZE; i++, at++) {
			if (at->at_flags == 0 || (at->at_flags & ATF_PERM))
				continue;
			if (++at->at_timer < ((at->at_flags&ATF_COM) ?
			    ARPT_KILLC : ARPT_KILLI))
				continue;
			/* timer has expired, clear entry */
			arptfree(at);
		}
	}
}

/*
 * Broadcast an ARP packet, asking who has addr on interface ac.
 */
arpwhohas(ac, addr)
	register struct arpcom *ac;
	struct in_addr *addr;
{
	register struct mbuf *m;
	register struct ether_header *eh;
	register struct ether_arp *ea;
	struct sockaddr sa;

	if ((m = m_get(M_DONTWAIT, MT_DATA)) == NULL)
		return (1);
	m->m_len = sizeof *ea;
	m->m_off = MMAXOFF - m->m_len;
	ea = mtod(m, struct ether_arp *);
	eh = (struct ether_header *)sa.sa_data;
	bzero((caddr_t)ea, sizeof (*ea));
	eh->ether_dhost = etherbroadcastaddr;
	eh->ether_type = ETHERPUP_ARPTYPE;	/* if_output will swap */
	ea->arp_hrd = htons(ARPHRD_ETHER);
	ea->arp_pro = htons(ETHERPUP_IPTYPE);
	ea->arp_hln = sizeof arp_sha(ea);	/* hardware address length */
	ea->arp_pln = sizeof arp_spa(ea);	/* protocol address length */
	ea->arp_op = htons(ARPOP_REQUEST);
	arp_sha(ea) = ac->ac_enaddr;
	arp_spa(ea) = ((struct sockaddr_in *)&ac->ac_if.if_addr)->sin_addr;
	arp_tpa(ea) = *addr;
	sa.sa_family = AF_UNSPEC;
	return ((*ac->ac_if.if_output)(&ac->ac_if, m, &sa));
}

/*
 * Resolve an IP address into an ethernet address.  If success, 
 * desten is filled in and 1 is returned.  If there is no entry
 * in arptab, set one up and broadcast a request 
 * for the IP address;  return 0.  Hold onto this mbuf and 
 * resend it once the address is finally resolved.
 *
 * We do some (conservative) locking here at splimp, since
 * arptab is also altered from input interrupt service (ecintr/ilintr
 * calls arpinput when ETHERPUP_ARPTYPE packets come in).
 */
arpresolve(ac, m, destip, desten)
	register struct arpcom *ac;
	struct mbuf *m;
	register struct in_addr *destip;
	register struct ether_addr *desten;
{
	register struct arptab *at;
	register struct ifnet *ifp;
	register int i;
	struct sockaddr_in sin;
	int s, lna;

	lna = in_lnaof(*destip);
	if (lna == INADDR_ANY) {	/* broadcast address */
		*desten = etherbroadcastaddr;
		return (1);
	}
	ifp = &ac->ac_if;
	/* if for us, then use software loopback driver */
	if (destip->s_addr ==
	    ((struct sockaddr_in *)&ifp->if_addr)-> sin_addr.s_addr) {
		sin.sin_family = AF_INET;
		sin.sin_addr = *destip;
		(void) looutput(&loif, m, (struct sockaddr *)&sin);
		/*
		 * We have to return 0 here so the ethernet driver will
		 * think arpresolve failed and return without sending
		 * the packet.
		 */
		return (0);
	}
#ifdef IPENADDR
	if (destip->s_net == 0) {	/* real IP address unknown */
		struct ether_addr *arpgeten(), *ap;
		if ((ap = arpgeten(destip)) == NULL)
			return (0);
		*desten = *ap;
		return (1);
	}
#endif IPENADDR
	s = splimp();
	ARPTAB_LOOK(at, destip->s_addr);
	if (at == 0) {			/* not found */
		if (ifp->if_flags & IFF_NOARP) {
			*desten = ac->ac_enaddr;
			desten->ether_addr_octet[3] = (lna >> 16) & 0x7f;
			desten->ether_addr_octet[4] = (lna >> 8) & 0xff;
			desten->ether_addr_octet[5] = lna & 0xff;
			splx(s);
			return (1);
		} else {
			at = arptnew(destip);
			at->at_hold = m;
			arpwhohas(ac, destip);
			splx(s);
			return (0);
		}
	}
	at->at_timer = 0;		/* restart the timer */
	if (at->at_flags & ATF_COM) {	/* entry IS complete */
		*desten = at->at_enaddr;
		splx(s);
		return (1);
	}
	/*
	 * There is an arptab entry, but no ethernet address
	 * response yet.  Replace the held mbuf with this
	 * latest one.
	 */
	if (at->at_hold)
		m_freem(at->at_hold);
	at->at_hold = m;
	arpwhohas(ac, destip);		/* ask again */
	splx(s);
	return (0);
}

/*
 * Called from ecintr/ilintr when ether packet type ETHERPUP_ARP
 * is received.  Algorithm is that given in RFC 826.
 * In addition, a sanity check is performed on the sender
 * protocol address, to catch impersonators.
 */
arpinput(ac, m)
	register struct arpcom *ac;
	struct mbuf *m;
{
	register struct ether_arp *ea;
	struct ether_header *eh;
	register struct arptab *at = 0;  /* same as "merge" flag */
	struct sockaddr_in sin;
	struct sockaddr sa;
	struct mbuf *mhold;
	struct in_addr isaddr,itaddr,myaddr;

	if (m->m_len < sizeof *ea)
		goto out;
	if (ac->ac_if.if_flags & IFF_NOARP)
		goto out;
	myaddr = ((struct sockaddr_in *)&ac->ac_if.if_addr)->sin_addr;
	ea = mtod(m, struct ether_arp *);
	if (ntohs(ea->arp_pro) != ETHERPUP_IPTYPE)
		goto out;
	isaddr = arp_spa(ea);
	itaddr = arp_tpa(ea);
	if (!bcmp((caddr_t)&arp_sha(ea), (caddr_t)&ac->ac_enaddr,
	  sizeof (ac->ac_enaddr)))
		goto out;	/* it's from me, ignore it. */
	if (isaddr.s_addr == myaddr.s_addr) {
		printf("duplicate IP address!! sent from ethernet address: ");
		ether_print(&arp_sha(ea));
		itaddr = myaddr;
		if (ntohs(ea->arp_op) == ARPOP_REQUEST) {
			ARPTAB_LOOK(at, itaddr.s_addr);
			if (at)
				goto reply;
		}
		goto out;
	}
	ARPTAB_LOOK(at, isaddr.s_addr);
	if (at) {		/* XXX ? - can overwrite ATF_PERM */
		at->at_enaddr = arp_sha(ea);
		at->at_flags |= ATF_COM;
		if (at->at_hold) {
			mhold = at->at_hold;
			at->at_hold = 0;
			sin.sin_family = AF_INET;
			sin.sin_addr = isaddr;
			(*ac->ac_if.if_output)(&ac->ac_if, 
			    mhold, (struct sockaddr *)&sin);
		}
	} else if (itaddr.s_addr == myaddr.s_addr) {
		/* ensure we have a table entry */
		at = arptnew(&isaddr);
		at->at_enaddr = arp_sha(ea);
		at->at_flags |= ATF_COM;
	}
	if (ntohs(ea->arp_op) != ARPOP_REQUEST)
		goto out;
	ARPTAB_LOOK(at, itaddr.s_addr);
	if (at == NULL) {
		if (itaddr.s_addr != myaddr.s_addr)
			goto out;	/* if I am not the target */
		at = arptnew(&myaddr);
		at->at_enaddr = ac->ac_enaddr;
		at->at_flags |= ATF_COM;
	} 
	if (itaddr.s_addr != myaddr.s_addr && (at->at_flags & ATF_PUBL) == 0)
		goto out;
		
reply:
	arp_tha(ea) = arp_sha(ea);
	arp_tpa(ea) = arp_spa(ea);
	arp_sha(ea) = at->at_enaddr;
	arp_spa(ea) = itaddr;
	ea->arp_op = htons(ARPOP_REPLY);
	eh = (struct ether_header *)sa.sa_data;
	eh->ether_dhost = arp_tha(ea);
	eh->ether_type = ETHERPUP_ARPTYPE;
	sa.sa_family = AF_UNSPEC;
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa);
	return;
out:
	m_freem(m);
	return;
}

#ifdef IPENADDR
/*
 * Called just before buffer passed to ipintr.  If ip_src.s_net is 0,
 * then make the ether source address available by saving the mapping.
 */
arpipin(ec, m)
	register struct ether_header *ec;
	register struct mbuf *m;
{
	register struct ip *ip;
	int hlen;
	extern int ipcksum;

	if (m->m_len < sizeof(struct ip))
		return;
	ip = mtod(m, struct ip *);
	if (ip->ip_src.s_net != 0)
		return;
	arpseten(&ip->ip_src, &ec->ether_shost, 0);
}

/*
 * Get an ethernet address, given the IP address.
 */
struct ether_addr *
arpgeten(addr)
	register struct in_addr *addr;
{
	register struct arptab *at;

	ARPTAB_LOOK(at, addr->s_addr);
	if (at == 0 || (at->at_flags & ATF_COM) == 0)
		return (0);
	return (&at->at_enaddr);
}

/*
 * Set an ether/IP mapping.  Called externally to force a mapping.
 */
arpseten(iaddr, eaddr, perm)
	struct in_addr *iaddr;
	struct ether_addr *eaddr;
{
	register struct arptab *at;
	int s = splimp();

	ARPTAB_LOOK(at, iaddr->s_addr);
	if (at == 0)
		at = arptnew(iaddr);
	if (at == 0) {
		splx(s);
		return (1);
	}
	if (perm) 
		at->at_flags |= ATF_COM+ATF_PERM;
	else {
		at->at_timer = ARPT_KILLC - 3;	/* expires faster than normal */
		at->at_flags |= ATF_COM;
	}
	if (at->at_hold)
		m_freem(at->at_hold);
	at->at_hold = 0;
	at->at_enaddr = *eaddr;
	splx(s);
	return (0);
}
#endif IPENADDR
	
/*
 * Free an arptab entry.
 */
arptfree(at)
	register struct arptab *at;
{
	int s = splimp();

	if (at->at_hold)
		m_freem(at->at_hold);
	at->at_hold = 0;
	at->at_timer = at->at_flags = 0;
	at->at_iaddr.s_addr = 0;
	splx(s);
}

/*
 * Enter a new address in arptab, pushing out the oldest entry 
 * from the bucket if there is no room.
 * This always succeeds for dynamic entries since arpioctl
 * gaurantees that no bucket can be completely filled
 * with permanent entries.
 */
struct arptab *
arptnew(addr)
	struct in_addr *addr;
{
	register n;
	int oldest = 0;
	register struct arptab *at, *ato;
	static int first = 1;

	if (first) {
		first = 0;
		timeout(arptimer, (caddr_t)0, hz);
	}
	at = &arptab[ARPTAB_HASH(addr->s_addr) * ARPTAB_BSIZ];
	ato = NULL;
	for (n = 0 ; n < ARPTAB_BSIZ ; n++,at++) {
		if (at->at_flags == 0)
			goto out;	 /* found an empty entry */
		if (at->at_flags & ATF_PERM)
			continue;
		if (ato == NULL || at->at_timer > oldest) {
			oldest = at->at_timer;
			ato = at;
		}
	}
	if (ato == NULL)
		return (NULL);
	at = ato;
	arptfree(at);
out:
	at->at_iaddr = *addr;
	at->at_flags = ATF_INUSE;
	return (at);
}

arpioctl(cmd, data)
	int cmd;
	caddr_t data;
{
	register struct arpreq *ar = (struct arpreq *)data;
	register struct arptab *at;
	register struct sockaddr_in *sin;
	int s;

	if (ar->arp_pa.sa_family != AF_INET ||
	    ar->arp_ha.sa_family != AF_UNSPEC)
		return (EAFNOSUPPORT);
	sin = (struct sockaddr_in *)&ar->arp_pa;
	s = splimp();
	ARPTAB_LOOK(at, sin->sin_addr.s_addr);
	if (at == NULL) {		/* not found */
		if (cmd != SIOCSARP) {
			splx(s);
			return (ENXIO);
		}
		if (if_ifwithnet(&ar->arp_pa) == NULL) {
			splx(s);
			return (ENETUNREACH);
		}
		at = arptnew(&sin->sin_addr);
	}
	switch (cmd) {

	case SIOCSARP:		/* set entry */
		at->at_enaddr = *(struct ether_addr *)ar->arp_ha.sa_data;
		at->at_flags = ATF_COM | ATF_INUSE |
			(ar->arp_flags & (ATF_PERM|ATF_PUBL));
		at->at_timer = 0;
		if (ar->arp_flags & ATF_PERM) {
			/* never make all entries in a bucket permanent */
			register struct arptab *tat;
			struct in_addr addr;
			
			/* defeat has and try to re-allocate */
			addr = at->at_iaddr;
			at->at_iaddr.s_addr = 0;
			tat = arptnew(&addr);
			if (tat == NULL) {
				arptfree(at);
				splx(s);
				return (EADDRNOTAVAIL);
			}
			arptfree(tat);
			at->at_iaddr = addr;
		}
		break;

	case SIOCDARP:		/* delete entry */
		arptfree(at);
		break;

	case SIOCGARP:		/* get entry */
		*(struct ether_addr *)ar->arp_ha.sa_data = at->at_enaddr;
		ar->arp_flags = at->at_flags;
		break;
	}
	splx(s);
	return (0);
}

#ifdef notdef
/* no revarp for this release */
struct in_addr myaddr;

/*
 * Called from when ether packet type ETHERPUP_REVARP
 * is received.  Algorithm is that given in RFC 903.
 * Someone wants to know their Internet address.
 * For now, we just scan the ARP table and hope its in there.
 * XXX !!! Linear search of LARGE table.
 */
revarpinput(ac, m)
	register struct arpcom *ac;
	struct mbuf *m;
{
	register struct ether_arp *ea;
	register u_char *p, *q;
	struct ether_header *eh;
	register struct arptab *at;
	struct sockaddr_in sin;
	struct sockaddr sa;

	ea = mtod(m, struct ether_arp *);
	if (m->m_len < sizeof *ea)
		goto out;
	if (ac->ac_if.if_flags & IFF_NOARP)
		goto out;
	if (ntohs(ea->arp_pro) != ETHERPUP_IPTYPE)
		goto out;
	if (ntohs(ea->arp_op) == REVARP_REPLY) {
		myaddr = arp_tpa(ea);
		wakeup((caddr_t)&myaddr);
		goto out;
	}
	if (ntohs(ea->arp_op) != REVARP_REQUEST)
		goto out;

	p = &arp_tha(ea).ether_addr_octet[0];
	for (at = arptab; at < &arptab[ARPTAB_SIZE]; at++) {
		if ((at->at_flags & ATF_COM) == 0)
			continue;
		if (at->at_iaddr.s_net == 0)	/* dummy ND entry */
			continue;
		q = &at->at_enaddr.ether_addr_octet[0];
		if (p[5] == q[5] && p[4] == q[4] && p[3] == q[3] &&
		    p[2] == q[2] && p[1] == q[1] && p[0] == q[0])
			break;
	}
	if (at >= &arptab[ARPTAB_SIZE])
		goto out;

	eh = (struct ether_header *)sa.sa_data;
	eh->ether_dhost = arp_sha(ea);
	eh->ether_type = ETHERPUP_REVARPTYPE;
	sa.sa_family = AF_UNSPEC;
	ea->arp_op = htons(REVARP_REPLY);
	arp_tpa(ea) = at->at_iaddr;
	arp_sha(ea) = ac->ac_enaddr;
	arp_spa(ea) = ((struct sockaddr_in *)&ac->ac_if.if_addr)->sin_addr;
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa);
	return;
out:
	m_freem(m);
	return;
}

/*
 * Revarp for clients is disabled until enough servers
 * implement it.  ND continues its hack for IP addresses
 * until revarp is enabled.
 */
int revarp = 0;
#endif notdef

revarp_myaddr(ifp)
	register struct ifnet *ifp;
{
	register struct sockaddr_in *sin;
	int s;

	/*
	 * We need to give the interface a temporary address just
	 * so it gets initialized. Hopefully, the address won't get used.
	 */
	sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_addr = if_makeaddr(INADDR_ANY, machineid() & 0xFFFFFF);
	if (if_setaddr(ifp, (struct sockaddr *)sin))
		printf("revarp: can't set temp inet addr\n");
#ifdef notdef
	if (revarp) {
		myaddr.s_addr = 0;
		revarp_start(ifp);
		s = splimp();
		while (myaddr.s_addr == 0)
			sleep((caddr_t)&myaddr, PZERO-1);
		splx(s);
		sin->sin_addr = myaddr;
		if (if_setaddr(ifp, (struct sockaddr *)sin))
			printf("revarp: can't set perm inet addr\n");
	}
#endif
}

#ifdef notdef
revarp_start(ifp)
	register struct ifnet *ifp;
{
	register struct mbuf *m;
	register struct ether_arp *ea;
	register struct ether_header *eh;
	static int retries = 0;
	struct ether_addr myether;
	struct sockaddr sa;

	if (myaddr.s_addr != 0) {
		if (retries >= 2)
			printf("Found Internet address %x\n", myaddr.s_addr);
		retries = 0;
		return;
	}
	localetheraddr(NULL, &myether);
	if (++retries == 2) {
		printf("revarp: Requesting Internet address for ");
		ether_print(&myether);
	}
	if ((m = m_get(M_DONTWAIT, MT_DATA)) == NULL)
		panic("revarp: no mbufs");
	m->m_len = sizeof(struct ether_arp);
	m->m_off = MMAXOFF - m->m_len;
	ea = mtod(m, struct ether_arp *);
	bzero((caddr_t)ea, sizeof (*ea));

	sa.sa_family = AF_UNSPEC;
	eh = (struct ether_header *)sa.sa_data;
	eh->ether_dhost = etherbroadcastaddr;
	eh->ether_shost = myether;
	eh->ether_type = ETHERPUP_REVARPTYPE;

	ea->arp_hrd = htons(ARPHRD_ETHER);
	ea->arp_pro = htons(ETHERPUP_IPTYPE);
	ea->arp_hln = sizeof arp_sha(ea);	/* hardware address length */
	ea->arp_pln = sizeof arp_spa(ea);	/* protocol address length */
	ea->arp_op = htons(REVARP_REQUEST);
	arp_sha(ea) = myether;
	arp_tha(ea) = myether;
	(*ifp->if_output)(ifp, m, &sa);
	timeout(revarp_start, ifp, 3*hz);
}
#endif

localetheraddr(hint, result)
	struct ether_addr *hint, *result;
{
	static int found = 0;
	static struct ether_addr addr;

	if (!found) {
		found = 1;
		if (hint == NULL)
			return (0);
		addr = *hint;
		printf("Ethernet address = ");
		ether_print(&addr);
	}
	if (result != NULL)
		*result = addr;
	return (1);
}

ether_print(ea)
	struct ether_addr *ea;
{
	u_char *cp = &ea->ether_addr_octet[0];

	printf("%x:%x:%x:%x:%x:%x\n", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
}
