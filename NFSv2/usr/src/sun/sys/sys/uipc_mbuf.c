/*	@(#)uipc_mbuf.c 1.1 85/05/30 SMI; from UCB 1.43 83/05/27	*/

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/cmap.h"
#include "../h/map.h"
#include "../h/mbuf.h"
#include "../h/vm.h"
#include "../h/kernel.h"

struct map mbmap[NMBCLUSTERS/4];
int mclbytes = 1024;
char mclrefcnt[NMBCLUSTERS*CLBYTES/1024];
#define	mtocl(x) (((int)x - (int)mbutl) / mclbytes)
caddr_t m_clalloc();

mbinit()
{
	rminit(mbmap, (long)((NMBCLUSTERS - 1) * CLSIZE), (long)CLSIZE,
	    "mbclusters", NMBCLUSTERS/4);

	if (m_clalloc(4096/mclbytes, MPG_MBUFS) == 0)
		goto bad;
	if (m_clalloc(8*4096/mclbytes, MPG_CLUSTERS) == 0)
		goto bad;
	return;
bad:
	panic("mbinit");
}

caddr_t
m_clalloc(ncl, how)
	register int ncl;
	int how;
{
	int npg, mbx;
	register struct mbuf *m;
	register int i;
	int s;

	npg = btoc(ncl * mclbytes);
	ncl = npg * NBPG / mclbytes;
	s = splimp();		/* careful: rmalloc isn't reentrant */
	mbx = rmalloc(mbmap, (long)npg);
	splx(s);
	if (mbx == 0)
		return (0);
	if (memall(&Mbmap[mbx], npg, &proc[0], CSYS) == 0) {
		s = splimp();
		rmfree(mbmap, (long)npg, (long)mbx);
		splx(s);
		return (0);
	}
	m = (struct mbuf *)((int)mbutl + ((mbx/CLSIZE) << CLSHIFT));
	vmaccess(&Mbmap[mbx], (caddr_t)m, npg);
	switch (how) {

	case MPG_CLUSTERS:
		s = splimp();
		for (i = 0; i < ncl; i++) {
			m->m_off = 0;
			m->m_next = mclfree;
			mclfree = m;
			m += mclbytes / sizeof (*m);
			mbstat.m_clfree++;
		}
		mbstat.m_clusters += ncl;
		splx(s);
		break;

	case MPG_MBUFS:
		for (i = ncl * mclbytes / sizeof (*m); i > 0; i--) {
			m->m_off = 0;
			m->m_type = MT_DATA;
			mbstat.m_mtypes[MT_DATA]++;
			mbstat.m_mbufs++;
			(void) m_free(m);
			m++;
		}
		break;

	case MPG_SPACE:		/* used by if_uba */
		break;
	}
	return ((caddr_t)m);
}

m_pgfree(addr, n)
	caddr_t addr;
	int n;
{

#ifdef lint
	addr = addr; n = n;
#endif
}

m_expand()
{

	if (m_clalloc(1, MPG_MBUFS) == 0)
		goto steal;
	return (1);
steal:
	/* should ask protocols to free code */
	return (0);
}

/* NEED SOME WAY TO RELEASE SPACE */

/*
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */
struct mbuf *
m_get(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	return (m);
}

struct mbuf *
m_getclr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	m = m_get(canwait, type);
	if (m == 0)
		return (0);
	bzero(mtod(m, caddr_t), MLEN);
	return (m);
}

struct mbuf *
m_free(m)
	struct mbuf *m;
{
	register struct mbuf *n;

	MFREE(m, n);
	return (n);
}

/*ARGSUSED*/
struct mbuf *
m_more(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	if (!m_expand()) {
		mbstat.m_drops++;
		return (NULL);
	}
#define m_more(x,y) (panic("m_more"), (struct mbuf *)0)
	MGET(m, canwait, type);
#undef m_more
	return (m);
}

m_freem(m)
	register struct mbuf *m;
{
	register struct mbuf *n;
	register int s;

	if (m == NULL)
		return;
	s = splimp();
	do {
		MFREE(m, n);
	} while (m = n);
	splx(s);
}

/*
 * Mbuffer utility routines.
 */
struct mbuf *
m_copy(m, off, len)
	register struct mbuf *m;
	int off;
	register int len;
{
	register struct mbuf *n, **np;
	struct mbuf *top, *p;
	int type;

	if (len == 0)
		return (0);
	if (off < 0 || len < 0)
		panic("m_copy");
	type = m->m_type;
	while (off > 0) {
		if (m == 0)
			panic("m_copy");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL)
				panic("m_copy");
			break;
		}
		MGET(n, M_WAIT, type);
		*np = n;
		if (n == 0)
			goto nospace;
		n->m_len = MIN(len, m->m_len - off);
		if (m->m_off > MMAXOFF && n->m_len > MLEN) {
			mcldup(m, n, off);
			n->m_off += off;
		} else
			bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			    (unsigned)n->m_len);
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	return (top);
nospace:
	m_freem(top);
	return (0);
}

m_cat(m, n)
	register struct mbuf *m, *n;
{
	while (m->m_next)
		m = m->m_next;
	while (n) {
		if (m->m_off >= MMAXOFF ||
		    m->m_off + m->m_len + n->m_len > MMAXOFF) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		    (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

m_adj(mp, len)
	struct mbuf *mp;
	register int len;
{
	register struct mbuf *m, *n;

	if ((m = mp) == NULL)
		return;
	if (len >= 0) {
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_off += len;
				break;
			}
		}
	} else {
		/* a 2 pass algorithm might be better */
		len = -len;
		while (len > 0 && m->m_len != 0) {
			while (m != NULL && m->m_len != 0) {
				n = m;
				m = m->m_next;
			}
			if (n->m_len <= len) {
				len -= n->m_len;
				n->m_len = 0;
				m = mp;
			} else {
				n->m_len -= len;
				break;
			}
		}
	}
}

struct mbuf *
m_pullup(m0, len)
	struct mbuf *m0;
	int len;
{
	register struct mbuf *m, *n;
	int count;

	n = m0;
	if (len > MLEN)
		goto bad;
	MGET(m, M_DONTWAIT, n->m_type);
	if (m == 0)
		goto bad;
	m->m_len = 0;
	do {
		count = MIN(MLEN - m->m_len, len);
		if (count > n->m_len)
			count = n->m_len;
		if (count > 0) {
			bcopy(mtod(n, caddr_t), mtod(m, caddr_t)+m->m_len,
			  (unsigned)count);
			len -= count;
			m->m_len += count;
			n->m_off += count;
			n->m_len -= count;
		}
		if (n->m_len > 0)
			break;
		n = m_free(n);
	} while (n);
	if (len) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}

mclget(m)
	register struct mbuf *m;
{
	int ms = splimp();
	register struct mbuf *p;

	if (p = mclfree) {
		++mclrefcnt[mtocl(p)];
		mbstat.m_clfree--;
		mclfree = p->m_next;
		m->m_len = mclbytes;
		m->m_off = (int)p - (int)m;
		m->m_cltype = 1;
	}
	splx(ms);
	return (p ? 1 : 0);
}

struct mbuf *
mclgetx(fun, arg, addr, len, wait)
	int (*fun)(), arg, len, wait;
	caddr_t addr;
{
	register struct mbuf *m;

	MGET(m, wait, MT_DATA);
	if (m == 0)
		return (0);
	m->m_off = (int)addr - (int)m;
	m->m_len = len;
	m->m_cltype = 2;
	m->m_clfun = fun;
	m->m_clarg = arg;
	m->m_clswp = NULL;
	return (m);
}

mclput(m)
	register struct mbuf *m;
{
	switch (m->m_cltype) {
	case 1:
		m = (struct mbuf *)(mtod(m, int) & ~(mclbytes - 1));
		if (--mclrefcnt[mtocl(m)] == 0) {
			m->m_next = mclfree;
			mclfree = m;
			mbstat.m_clfree++;
		}
		break;

	case 2:
		(*m->m_clfun)(m->m_clarg);
		break;

	default:
		panic("mclput");
	}
}

static
buffree(arg)
	int arg;
{
	kmem_free_intr(arg, *(int *)arg);
}

mcldup(m, n, off)
	register struct mbuf *m, *n;
	int off;
{
	register struct mbuf *p;
	register caddr_t copybuf;

	switch (m->m_cltype) {
	case 1:
		p = mtod(m, struct mbuf *);
		n->m_off = (int)p - (int)n;
		n->m_cltype = 1;
		mclrefcnt[mtocl(p)]++;
		break;
	case 2:
		copybuf = (caddr_t)kmem_alloc(n->m_len + sizeof (int));
		* (int *) copybuf = n->m_len + sizeof (int);
		bcopy(mtod(m, caddr_t) + off, copybuf + sizeof (int),n->m_len);
		n->m_off = (int)copybuf + sizeof (int) - (int)n - off;
		n->m_cltype = 2;
		n->m_clfun = buffree;
		n->m_clarg = (int)copybuf;
		n->m_clswp = NULL;
		break;
	default:
		panic("mcldup");
	}
}

/*
 * Move an mbuf chain to contiguous locations.
 * Checks for possibility of page exchange to accomplish move
 * Free chain when moved
 */
m_movtoc(m, to, count)
	register struct mbuf *m;
	register caddr_t to;
	register int count;
{
	register struct mbuf *m0;
	register caddr_t from;
	register int i;

	while (m != NULL) {
		i = MIN(m->m_len, count);
		from = mtod(m, caddr_t);
		if (i >= mclbytes && m->m_cltype == 2 && m->m_clswp &&
		    (((int)from | (int)to) & (mclbytes-1)) == 0 &&
		    (*m->m_clswp)(m->m_clarg, from, to)) {
			i -= mclbytes;
			from += mclbytes;
			to += mclbytes;
		}
		if (i > 0) {
			bcopy(from, to, i);
			count -= i;
			to += i;
		}
		m0 = m;
		MFREE(m0, m);
	}
	return (count);
}
