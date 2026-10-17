#ifndef lint
static char sccsid[] = "@(#)svc.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * svc.c, Server-side remote procedure call interface.
 *
 * There are two sets of procedures here.  The xprt routines are
 * for handling transport handles.  The svc routines handle the
 * list of service routines.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#ifdef KERNEL
#include "../h/param.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../netinet/in.h"
#include "../rpc/types.h"
#include "../rpc/xdr.h"
#include "../rpc/auth.h"
#include "../rpc/clnt.h"
#include "../rpc/rpc_msg.h"
#include "../rpc/svc.h"
#include "../rpc/svc_auth.h"
#include "../h/time.h"
#else
#include <stdio.h>
#include "types.h"	/* <> */
#include <sys/errno.h>
#include <netinet/in.h>
#include "xdr.h"	/* <> */
#include "auth.h"	/* <> */
#include "clnt.h"	/* <> */
#include "rpc_msg.h"	/* <> */
#include "svc.h"	/* <> */
#include "svc_auth.h"	/* <> */
#include "pmap_clnt.h"	/* <make kernel depend happy> */

#define NOFILE 32

static SVCXPRT *xports[NOFILE];
int svc_fds = 0;
extern errno;
char *malloc();
#endif

#define NULL_SVC ((struct svc_callout *)0)

void rpctest_service();

/*
 * The services list
 * Each entry represents a set of procedures (an rpc program).
 * The dispatch routine takes request structs and runs the
 * apropriate procedure.
 */
static struct svc_callout {
	struct svc_callout *sc_next;
	u_long		    sc_prog;
	u_long		    sc_vers;
	void		    (*sc_dispatch)();
} *svc_head = NULL_SVC;

static struct svc_callout *svc_find();

/* ***************  SVCXPRT related stuff **************** */

/*
 * Activate a transport handle.
 */
void
xprt_register(xprt)
	SVCXPRT *xprt;
{
#ifndef KERNEL
	register int sock = xprt->xp_sock;

	if (sock < NOFILE) {
		xports[sock] = xprt;
		svc_fds |= (1 << sock);
	}
#endif
}

/*
 * De-activate a transport handle. 
 */
void
xprt_unregister(xprt) 
	SVCXPRT *xprt;
{ 
#ifndef KERNEL
	register int sock = xprt->xp_sock;

	if ((sock < NOFILE) && (xports[sock] == xprt)) {
		xports[sock] = (SVCXPRT *)NULL;
		svc_fds &= ~(1 << sock);
	}
#endif
} 


/* ********************** CALLOUT list related stuff ************* */

/*
 * Add a service program to the callout list.
 * The dispatch routine will be called when a rpc request for this
 * program number comes in.
 */
bool_t
svc_register(xprt, prog, vers, dispatch, protocol)
	SVCXPRT *xprt;
	u_long prog;
	u_long vers;
	void (*dispatch)();
	int protocol;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

	if ((s = svc_find(prog, vers, &prev)) != NULL_SVC) {
		if (s->sc_dispatch == dispatch)
			goto pmap_it;  /* he is registering another xptr */
		return (FALSE);
	}
	s = (struct svc_callout *)mem_alloc(sizeof(struct svc_callout));
#ifndef KERNEL
	if (s == NULL) {
		fprintf(stderr, "svc_register: out of memory\n");
		return (FALSE);
	}
#endif
	s->sc_prog = prog;
	s->sc_vers = vers;
	s->sc_dispatch = dispatch;
	s->sc_next = svc_head;
	svc_head = s;
pmap_it:
#ifndef KERNEL
	/* now register the information with the local binder service */
	if (protocol) {
		return (pmap_set(prog, vers, protocol, xprt->xp_port));
	}
#endif
	return (TRUE);
}

/*
 * Remove a service program from the callout list.
 */
void
svc_unregister(prog, vers)
	u_long prog;
	u_long vers;
{
	struct svc_callout *prev;
	register struct svc_callout *s;

	if ((s = svc_find(prog, vers, &prev)) == NULL_SVC)
		return;
	if (prev == NULL_SVC) {
		svc_head = s->sc_next;
	} else {
		prev->sc_next = s->sc_next;
	}
	s->sc_next = NULL_SVC;
	mem_free(s, sizeof(struct svc_callout));
#ifndef KERNEL
	/* now unregister the information with the local binder service */
	(void)pmap_unset(prog, vers);
#endif
}

/*
 * Search the callout list for a program number, return the callout
 * struct.
 */
static struct svc_callout *
svc_find(prog, vers, prev)
	u_long prog;
	u_long vers;
	struct svc_callout **prev;
{
	register struct svc_callout *s, *p;

	p = NULL_SVC;
	for (s = svc_head; s != NULL_SVC; s = s->sc_next) {
		if ((s->sc_prog == prog) && (s->sc_vers == vers))
			goto done;
		p = s;
	}
done:
	*prev = p;
	return (s);
}

/* ******************* REPLY GENERATION ROUTINES  ************ */

/*
 * Send a reply to an rpc request
 */
bool_t
svc_sendreply(xprt, xdr_results, xdr_location)
	register SVCXPRT *xprt;
	xdrproc_t xdr_results;
	caddr_t xdr_location;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY;  
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf; 
	rply.acpted_rply.ar_stat = SUCCESS;
	rply.acpted_rply.ar_results.where = xdr_location;
	rply.acpted_rply.ar_results.proc = xdr_results;
	return (SVC_REPLY(xprt, &rply)); 
}

/*
 * No procedure error reply
 */
void
svcerr_noproc(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_ACCEPTED;
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = PROC_UNAVAIL;
	SVC_REPLY(xprt, &rply);
}

/*
 * Can't decode args error reply
 */
void
svcerr_decode(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY; 
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = GARBAGE_ARGS;
	SVC_REPLY(xprt, &rply); 
}

/*
 * Some system error
 */
void
svcerr_systemerr(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply; 

	rply.rm_direction = REPLY; 
	rply.rm_reply.rp_stat = MSG_ACCEPTED; 
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = SYSTEM_ERR;
	SVC_REPLY(xprt, &rply); 
}

/*
 * Authentication error reply
 */
void
svcerr_auth(xprt, why)
	SVCXPRT *xprt;
	enum auth_stat why;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_DENIED;
	rply.rjcted_rply.rj_stat = AUTH_ERROR;
	rply.rjcted_rply.rj_why = why;
	SVC_REPLY(xprt, &rply);
}

/*
 * Auth too weak error reply
 */
void
svcerr_weakauth(xprt)
	SVCXPRT *xprt;
{

	svcerr_auth(xprt, AUTH_TOOWEAK);
}

/*
 * Program unavailable error reply
 */
void 
svcerr_noprog(xprt)
	register SVCXPRT *xprt;
{
	struct rpc_msg rply;  

	rply.rm_direction = REPLY;   
	rply.rm_reply.rp_stat = MSG_ACCEPTED;  
	rply.acpted_rply.ar_verf = xprt->xp_verf;  
	rply.acpted_rply.ar_stat = PROG_UNAVAIL;
	SVC_REPLY(xprt, &rply);
}

/*
 * Program version mismatch error reply
 */
void  
svcerr_progvers(xprt, low_vers, high_vers)
	register SVCXPRT *xprt; 
	u_long low_vers;
	u_long high_vers;
{
	struct rpc_msg rply;

	rply.rm_direction = REPLY;
	rply.rm_reply.rp_stat = MSG_ACCEPTED;
	rply.acpted_rply.ar_verf = xprt->xp_verf;
	rply.acpted_rply.ar_stat = PROG_MISMATCH;
	rply.acpted_rply.ar_vers.low = low_vers;
	rply.acpted_rply.ar_vers.high = high_vers;
	SVC_REPLY(xprt, &rply);
}

/* ******************* SERVER INPUT STUFF ******************* */

/*
 * Get server side input from some transport
 */
void
#ifdef KERNEL
svc_getreq(xprt)
	register SVCXPRT *xprt;
#else
svc_getreq(rdfds)
	int rdfds;
#endif
{
#ifndef KERNEL
	register int sock;
	register int readfds = rdfds & svc_fds;
	register SVCXPRT *xprt;
#endif
	register enum xprt_stat stat;
	struct rpc_msg msg;
	/*char cred_body[MAX_AUTH_BYTES], verf_body[MAX_AUTH_BYTES];*/
	char *cred_body;
	int prog_found;
	u_long low_vers;
	u_long high_vers;

	cred_body = (char *)mem_alloc(2 * MAX_AUTH_BYTES);
#ifndef KERNEL
	if (cred_body == NULL) {
		fprintf(stderr, "svc_getreq: out of memory\n");
		svcerr_systemerr(xprt);
		goto call_done;
	}
#endif
	msg.rm_call.cb_cred.oa_base = cred_body;
	msg.rm_call.cb_verf.oa_base = &(cred_body[MAX_AUTH_BYTES]);
#ifndef KERNEL
	for (sock = 0; readfds != 0; sock++, readfds >>= 1) {
	    if ((readfds & 1) != 0) {
		/* sock has input waiting */
		xprt = xports[sock];
		/* now receive msgs from xprtprt (support batch calls) */
#endif
		do {
			if (SVC_RECV(xprt, &msg)) {

				/* now find the exported program and call it */
				register struct svc_callout *s;
				enum auth_stat why;
				struct svc_req r;

				r.rq_xprt = xprt;
				r.rq_prog = msg.rm_call.cb_prog;
				r.rq_vers = msg.rm_call.cb_vers;
				r.rq_proc = msg.rm_call.cb_proc;
				r.rq_cred = msg.rm_call.cb_cred;
				/* first authenticate the message */
				if ((why= _authenticate(&r, &msg)) != AUTH_OK) {
					svcerr_auth(xprt, why);
					goto call_done;
				}
				/* now match message with a registered service*/
				prog_found = FALSE;
				low_vers = 0 - 1;
				high_vers = 0;
				for (s = svc_head; s != NULL_SVC; s = s->sc_next) {
					if (s->sc_prog == r.rq_prog) {
						if (s->sc_vers == r.rq_vers) {
							(*s->sc_dispatch)(&r, xprt);
							goto call_done;
						}  /* found correct version */
						prog_found = TRUE;
						if (s->sc_vers < low_vers)
							low_vers = s->sc_vers;
						if (s->sc_vers > high_vers)
							high_vers = s->sc_vers;
					}   /* found correct program */
				}
				/*
				 * if we got here, the program or version
				 * is not served ...
				 */
				if (prog_found)
					svcerr_progvers(xprt,
					low_vers, high_vers);
				else
					 svcerr_noprog(xprt);
				/* Fall through to ... */
			}
		call_done:
			if ((stat = SVC_STAT(xprt)) == XPRT_DIED){
				SVC_DESTROY(xprt);
				break;
			}
		} while (stat == XPRT_MOREREQS);
#ifndef KERNEL
	    }
	}
#endif
	mem_free(cred_body, 2 * MAX_AUTH_BYTES);
}

/*
 * This is the rpc server side idle loop
 * Wait for input, call server program.
 */
#ifdef KERNEL
int Rpccnt = 0;
void
svc_run(xprt)
	SVCXPRT *xprt;
{
	int	s;

	while (TRUE) {
		s = splnet();
		while (xprt->xp_sock->so_rcv.sb_cc == 0)
			sbwait(&xprt->xp_sock->so_rcv);
		splx(s);
		svc_getreq(xprt);
		Rpccnt++;
	}
}
#else
void
svc_run()
{
	int readfds;

	while (TRUE) {
		readfds = svc_fds;
		switch (select(32, &readfds, (int *)NULL, (int *)NULL,
		    (struct timeval *)NULL)) {

		case -1:
                        if (errno == EINTR)
                                continue;
	                else {
                                perror("svc.c: - Select failed");
                                return;
			}
		case 0:
			continue;
		default:
			svc_getreq(readfds);
		}
	}
}
#endif

/* *************************** BUILTIN TEST SERVICE **************** */
void
rpctest_service(reqst, xprt)
	struct svc_req *reqst;
	register SVCXPRT *xprt;
{

	switch (reqst->rq_proc) {

	case RPCTEST_NULL_PROC:
		/* get nullp parameters, return null results */
		if (SVC_GETARGS(xprt, xdr_void, NULL)) {
			svc_sendreply(xprt, (xdrproc_t)xdr_void, (caddr_t)NULL);
		} else {
			svcerr_decode(xprt);
		}
		return;

	case  RPCTEST_NULL_BATCH_PROC:  /* nothing !! */
		return;

	default:
		svcerr_noproc(xprt);
		return;
	}
}
