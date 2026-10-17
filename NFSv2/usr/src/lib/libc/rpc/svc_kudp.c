/*      @(#)svc_kudp.c 1.1 85/05/30 SMI      */

/*
 * svc_kudp.c,
 * Server side for UDP/IP based RPC in the kernel.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include "../h/param.h"
#include "../rpc/types.h"
#include "../netinet/in.h"
#include "../rpc/xdr.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../rpc/rpc_msg.h"
#include "../rpc/svc.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/mbuf.h"

char *kmem_alloc();

#define rpc_buffer(xprt) ((xprt)->xp_p1)

/*
 * Routines exported through ops vector.
 */
bool_t		svckudp_recv();
bool_t		svckudp_send();
enum xprt_stat	svckudp_stat();
bool_t		svckudp_getargs();
bool_t		svckudp_freeargs();
void		svckudp_destroy();

/*
 * Server transport operations vector.
 */
struct xp_ops svckudp_op = {
	svckudp_recv,		/* Get requests */
	svckudp_stat,		/* Return status */
	svckudp_getargs,	/* Deserialize arguments */
	svckudp_send,		/* Send reply */
	svckudp_freeargs,	/* Free argument data space */
	svckudp_destroy		/* Destroy transport handle */
};

#ifdef RPCUDPCACHE
bool_t		cache_hit();
void		fill_cache();
void		end_call();
#endif
struct mbuf	*ku_recvfrom();

/*
 * Transport private data.
 * Kept in xprt->xp_p2.
 */
struct udp_data {
	u_long 	ud_xid;				/* id */
	struct mbuf *ud_inmbuf;			/* input mbuf chain */
	XDR	ud_xdrin;			/* input xdr stream */
	XDR	ud_xdrout;			/* output xdr stream */
	char	ud_verfbody[MAX_AUTH_BYTES];	/* verifier */
};


/*
 * Server statistics
 */
struct {
	int	rscalls;
	int	rsbadcalls;
	int	rsnullrecv;
	int	rsbadlen;
	int	rsxdrcall;
} rsstat;

/*
 * Create a transport record.
 * The transport record, output buffer, and private data structure
 * are allocated.  The output buffer is serialized into using xdrmem.
 * There is one transport record per user process which implements a
 * set of services.
 */
SVCXPRT *
svckudp_create(sock, port)
	struct socket	*sock;
	u_short		 port;
{
	register SVCXPRT	 *xprt;
	register struct udp_data *ud;

	rpc_debug(4, "svckudp_create so = %x, port = %d\n", sock, port);
	xprt = (SVCXPRT *)kmem_alloc(sizeof(SVCXPRT));
	rpc_buffer(xprt) = kmem_alloc(UDPMSGSIZE);
	ud = (struct udp_data *)kmem_alloc(sizeof(struct udp_data));
	ud->ud_inmbuf = (struct mbuf *)0;
	xprt->xp_addrlen = 0;
	xprt->xp_p2 = (caddr_t)ud;
	xprt->xp_verf.oa_base = ud->ud_verfbody;
	xprt->xp_ops = &svckudp_op;
	xprt->xp_port = port;
	xprt->xp_sock = sock;
	xprt_register(xprt);
	return (xprt);
}
 
/*
 * Destroy a transport record.
 * Frees the space allocated for a transport record.
 */
void
svckudp_destroy(xprt)
	register SVCXPRT   *xprt;
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;

	rpc_debug(4, "usr_destroy %x\n", xprt);
	mem_free((caddr_t)ud, sizeof(struct udp_data));
	mem_free(rpc_buffer(xprt), UDPMSGSIZE);
	mem_free((caddr_t)xprt, sizeof(SVCXPRT));
}

/*
 * Receive rpc requests.
 * Pulls a request in off the socket, checks if the packet is intact,
 * and deserializes the call packet.
 */
bool_t
svckudp_recv(xprt, msg)
	register SVCXPRT	 *xprt;
	struct rpc_msg		 *msg;
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	register XDR	 *xdrs = &(ud->ud_xdrin);
	register struct mbuf	 *m;
	int			  s;

	rpc_debug(4, "svckudp_recv %x\n", xprt);
	rsstat.rscalls++;
	s = splnet();
	m = ku_recvfrom(xprt->xp_sock, &(xprt->xp_raddr));
	splx(s);
	if (m == NULL) {
		rsstat.rsnullrecv++;
		return (FALSE);
	}

	if (m->m_len < 4*sizeof(u_long)) {
		rsstat.rsbadlen++;
		goto bad;
	}
	xdrmbuf_init(&ud->ud_xdrin, m, XDR_DECODE);
	if (! xdr_callmsg(xdrs, msg)) {
		rsstat.rsxdrcall++;
		goto bad;
	}
#ifdef RPCUDPCACHE
	if (cache_hit(xprt, msg)) {
		printf("svckudp_recv: cache_hit\n");
		goto bad;
	}
#endif
	ud->ud_xid = msg->rm_xid;
	ud->ud_inmbuf = m;
	rpc_debug(5, "svckudp_recv done\n");
	return (TRUE);

bad:
	m_freem(m);
	rsstat.rsbadcalls++;
	return (FALSE);
}

static
noop()
{
}

/*
 * Send rpc reply.
 * Serialize the reply packet into the output buffer then
 * call ku_sendto to make an mbuf out of it and send it.
 */
bool_t
/* ARGSUSED */
svckudp_send(xprt, msg)
	register SVCXPRT *xprt; 
	struct rpc_msg *msg; 
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	register XDR *xdrs = &(ud->ud_xdrout);
	register int slen;
	register int stat = FALSE;
	struct mbuf *m, *mclgetx();

	rpc_debug(4, "svckudp_send %x\n", xprt);
	m = mclgetx(noop, 0, rpc_buffer(xprt), UDPMSGSIZE, M_WAIT);
	if (m == NULL) {
		return (stat);
	}
	xdrmbuf_init(&ud->ud_xdrout, m, XDR_ENCODE);
	msg->rm_xid = ud->ud_xid;
	if (xdr_replymsg(xdrs, msg)) {
		slen = (int)XDR_GETPOS(xdrs);
#ifdef RPCUDPCACHE
		fill_cache(xprt, rpc_buffer(xprt), slen);
#endif
		if (m->m_next == 0) {		/* XXX */
			m->m_len = slen;
		}
		if (!ku_sendto_mbuf(xprt->xp_sock, m, &xprt->xp_raddr))
			stat = TRUE;
		/*
		 * This is completely disgusting.  If public is set it is
		 * a pointer to a structure whose first field is the address
		 * of the function to free that structure and any related
		 * stuff.  (see rrokfree in nfs_xdr.c).
		 */
		if (xdrs->x_public) {
			(**((int (**)())xdrs->x_public))(xdrs->x_public);
		}
	}
	rpc_debug(5, "svckudp_send done\n");
	return (stat);
}

/*
 * Return transport status.
 */
enum xprt_stat
svckudp_stat(xprt)
	SVCXPRT *xprt;
{

#ifdef RPCUDPCACHE
	end_call(xprt);
#endif
	return (XPRT_IDLE); 
}

/*
 * Deserialize arguments.
 */
bool_t
svckudp_getargs(xprt, xdr_args, args_ptr)
	SVCXPRT	*xprt;
	xdrproc_t	 xdr_args;
	caddr_t		 args_ptr;
{

	return ((*xdr_args)(&(((struct udp_data *)(xprt->xp_p2))->ud_xdrin), args_ptr));
}

bool_t
svckudp_freeargs(xprt, xdr_args, args_ptr)
	SVCXPRT	*xprt;
	xdrproc_t	 xdr_args;
	caddr_t		 args_ptr;
{
	register XDR *xdrs =
	    &(((struct udp_data *)(xprt->xp_p2))->ud_xdrin);
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;

	if (ud->ud_inmbuf) {
		m_freem(ud->ud_inmbuf);
	}
	ud->ud_inmbuf = (struct mbuf *)0;
	xdrs->x_op = XDR_FREE;
	return ((*xdr_args)(xdrs, args_ptr));
}

#ifdef RPCUDPCACHE
/*
 * "reply cache" code.
 * By remebering the last n replies, we can (almost) guarantee
 * execute-at-most semantics of the rpc.
 * There also exists a "in progress" cache so that duplicate incoming
 * messages can be filtered while the first message is being acted upon.
 * (This only makes sense in the light-weight process context.)
 */

#define CACHESIZE 5
#define INPROGRESSIZE 1

int in_finger = 0;
u_long cache_calls, cache_hits;

struct cache_item {
	u_long		ci_xid;
	struct sockaddr_in xp_raddr;
	int replylen;
	caddr_t reply;
	int max_replylen;
	u_long time_stamp;   /* but it's not clock time */
} null_cache_item, cache[CACHESIZE], in_progress[INPROGRESSIZE];

/* 
 * cache_hit returns TRUE if there was a cache hit.  The caller does no
 * more processing if cache_hit returns TRUE.  It is cache_hit responsibility
 * to send a reply packet of there was a cache hit.
 */
bool_t 
cache_hit(xprt, msg)
	register SVCXPRT *xprt;
	register struct rpc_msg *msg;
{
	register int i;
	register int blen = sizeof(struct sockaddr_in) + 2*sizeof(u_long) + sizeof(int);
	struct cache_item reqstitem;
	register struct cache_item *item, *rq;

	rq = &reqstitem;
	rq->ci_xid = msg->rm_xid;
	rq->xp_raddr = xprt->xp_raddr;
	rq->replylen = 0;
	cache_calls ++;
	for (i = 0, item = in_progress; i < in_finger; item++, i++) {
		if (bcmp((caddr_t)rq, (caddr_t)item, blen) == 0) {
			cache_hits ++;
			return (TRUE);  /* somebody else if working on it */
		}
	}
	for (i = 0, item = cache; i < CACHESIZE; item++, i++) {
		if (bcmp((caddr_t)rq, (caddr_t)item, blen) == 0) {
			cache_hits ++;
			(void)ku_sendto(xprt->xp_sock, item->reply,
			    item->replylen, &(item->xp_raddr));
			item->time_stamp = cache_calls;
			return (TRUE);
		}
	}
	in_progress[in_finger ++] = *rq; 
	return (FALSE);
}

void
fill_cache(xprt, buff, bufflen)
	register SVCXPRT *xprt;
	caddr_t buff;
	int bufflen;
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	register int i;
	register struct cache_item *item;
	register u_long trial_delta_t;
	register u_long max_delta_t = 0;
	int oldest_item = 0;
	int len;

	for (i = 0, item = cache; i < CACHESIZE; item++, i++) {

		if (bcmp((caddr_t)&xprt->xp_raddr,
		    (caddr_t)&item->xp_raddr,
		    sizeof(struct sockaddr_in)) == 0) {
			goto replace_it;
		}
		trial_delta_t = cache_calls - item->time_stamp;
		if (trial_delta_t > max_delta_t) {
			max_delta_t = trial_delta_t;
			oldest_item = i;
		}
	}
	item = &(cache[i = oldest_item]);

replace_it:
	/* recent cache entries should bubble to the top */
	if (i != 0) {
		caddr_t i_addr = item->reply;
		len = item->max_replylen;
		*item = cache[-- i];
		item -- ;
		item->reply = i_addr;
		item->max_replylen = len;
	}
	if (item->max_replylen < bufflen) {
		if (item->reply != NULL)
		    mem_free(item->reply, item->max_replylen);
		item->reply =
		    mem_free(item->max_replylen = bufflen);
	}
	bcopy(buff, item->reply, (u_int)(item->replylen = bufflen));
	item->ci_xid = ud->ud_xid;
	item->xp_raddr = xprt->xp_raddr;
	item->time_stamp = cache_calls;
}

void
end_call(xprt)
	register SVCXPRT *xprt;
{
	register struct udp_data *ud = (struct udp_data *)xprt->xp_p2;
	register int i;
	register struct cache_item *item;

	for (i = 0, item = in_progress; i < in_finger; item++, i++) {
		if ((ud->ud_xid == item->ci_xid) &&
		    (xprt->xp_raddr.sin_port == item->xp_raddr.sin_port) &&
		    (xprt->xp_raddr.sin_addr.s_addr ==
			item->xp_raddr.sin_addr.s_addr)) {
		*item = in_progress[-- in_finger];
		in_progress[in_finger] = null_cache_item;
		return;
		}
	}
}
#endif
