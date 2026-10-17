#ifndef lint
static	char sccsid[] = "@(#)ypbind.c 1.1 85/05/30 Copyr 1985 Sun Micro";
#endif

/*
 * This constructs a list of servers by domains, and keeps more-or-less up to
 * date track of those server's reachability.
  */

#include <dbm.h>			/* Pull this in first */
#undef NULL				/* Remove dbm.h's definition of NULL */
extern void dbmclose();			/* Refer to dbm routine not in dbm.h */
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <rpc/rpc.h>
#include <sys/dir.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <rpc/pmap_clnt.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

/*
 * The domain struct is the data structure used by the yp binder to remember
 * mappings of domain to a server.  The list of domains is pointed to by
 * known_domains.  Domains are added when the yp binder gets binding requests
 * for domains which are not currently on the list.  Once on the list, an
 * entry stays on the list forever.  Bindings are initially made by means of
 * a broadcast method, using functions ypbind_broadcast_bind and
 * ypbind_broadcast_ack.  This means of binding is re-done any time the domain
 * becomes unbound, which happens when a server doesn't respond to a ping.
 * current_domain is used to communicate among the various functions in this
 * module; it is set by ypbind_get_binding.
 */
struct domain {
	struct domain *dom_pnext;
	char dom_name[MAXNAMLEN + 1];
	bool dom_boundp;
	struct in_addr dom_serv_addr;
	unsigned short int dom_serv_port;
	int dom_report_success;		/* Controls msg to /dev/console*/
};
struct domain *known_domains = (struct domain *) NULL;
struct domain *current_domain;		/* Used by ypbind_broadcast_ack, set
					 *    by all callers of clnt_broadcast */
struct domain *broadcast_domain;	/* Set by ypbind_get_binding, used
					 *   by the mainline. */
bool broadcast_done = FALSE;		/* Set TRUE by the interrupt handler,
					 *   FALSE by the mainline. */
bool broadcast_binding_succeeded;	/* Set TRUE or FALSE or by the interrupt
					 *   handler, used by the mainline. */
int broadcaster_pid = 0;		/* Set non-zero by ypbind_get_binding,
					 *   0 by the mainline. */
SVCXPRT *tcphandle;
SVCXPRT *udphandle;

#define BINDING_TRIES 4			/* Number of times we'll broadcast to
					 *   try to bind default domain.  */
#ifdef  DEBUG
#define YPTIMEOUT 120			/* Total seconds for timeout */
#else
#define YPTIMEOUT 9			/* Total seconds for timeout */
#endif
static struct timeval total_timeout = {
	YPTIMEOUT,			/* Seconds */
	0				/* Microseconds */
	};
#define SETDOMINTERTRY 20
#define SETDOMTOTTIM 60
#ifdef DEVELOPMENT
int silent = FALSE;
#else
int silent = TRUE;
#endif

extern int svc_fds;
extern int errno;

void ypbind_dispatch();
void ypbind_get_binding();
void ypbind_set_binding();
void ypbind_send_setdom();
struct domain *ypbind_point_to_domain();
bool ypbind_broadcast_ack();
void ypbind_ping();
void ypbind_init_default();
void broadcast_proc_exit();

extern bool xdr_ypdomain_wrap_string();
extern bool xdr_ypbind_resp();

main()
{
	int pid;
	int t;
	int readfds;
	char *pname;
	bool true;

	pmap_unset(YPBINDPROG, YPBINDVERS);
	ypbind_init_default();

	if (silent) {
		
		pid = fork();
		
		if (pid == -1) {
			fprintf(stderr, "ypbind:  fork failure.\n");
			fflush(stderr);
			abort();
		}
	
		if (pid != 0) {
			exit(0);
		}
	
		for (t = 0; t < 20; t++) {
			close(t);
		}
	
 		open("/", 0);
 		dup2(0, 1);
 		dup2(0, 2);
 		t = open("/dev/tty", 2);
	
 		if (t >= 0) {
 			ioctl(t, TIOCNOTTY, (char *)0);
 			close(t);
 		}
	}

	if ((int) signal(SIGCHLD, broadcast_proc_exit) == -1) {
		fprintf(stderr,
		    "ypbind:  Can't catch broadcast process exit signal.\n");
		fflush(stderr);
		abort();
	}

	if ((tcphandle = svctcp_create(RPC_ANYSOCK, 1024, 1024)) == NULL) {
		fprintf(stderr, "ypbind:  can't create tcp service.\n");
		fflush(stderr);
		abort();
	}

	if (!svc_register(tcphandle, YPBINDPROG, YPBINDVERS,
	    ypbind_dispatch, IPPROTO_TCP) ) {
		fprintf(stderr, "ypbind:  can't register tcp service.\n");
		fflush(stderr);
		abort();
	}

	if ((udphandle = svcudp_create(RPC_ANYSOCK)) == (SVCXPRT *) NULL) {
		fprintf(stderr, "ypbind:  can't create udp service.\n");
		fflush(stderr);
		abort();
	}

	if (!svc_register(udphandle, YPBINDPROG, YPBINDVERS,
	    ypbind_dispatch, IPPROTO_UDP) ) {
		fprintf(stderr, "ypbind:  can't register udp service.\n");
		fflush(stderr);
		abort();
	}

	for (;;) {

		if (broadcast_done) {
			broadcast_done = FALSE;
			broadcaster_pid = 0;
			current_domain = broadcast_domain;

			if (broadcast_binding_succeeded) {
				current_domain->dom_report_success = -1;
			}
		}

		readfds = svc_fds;
		errno = 0;

		switch ( (int) select(32, &readfds, NULL, NULL, NULL) ) {

		case -1:  {
		
			if (errno != EINTR) {
			    fprintf (stderr,
			    "ypbind:  bad fds bits in main loop select mask.\n");
			}

			break;
		}

		case 0:  {
			fprintf (stderr,
			    "ypbind:  invalid timeout in main loop select.\n");
			break;
		}

		default:  {
			svc_getreq (readfds);
			break;
		}
		
		}
	}
}

/*
 * This dispatches to server action routines based on the input procedure number.
 * Called from rpc library level upon the receipt of a packet asking for 
 * services from program YPPROG and version YPVERS.
 */
void
ypbind_dispatch(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{

	switch (rqstp->rq_proc) {

	case YPBINDPROC_NULL:

		if (!svc_sendreply(transp, xdr_void, 0) ) {
			fprintf(stderr, "ypbind:  Can't reply to rpc call.\n");
		}

		break;

	case YPBINDPROC_DOMAIN:
		ypbind_get_binding(rqstp, transp);
		break;

	case YPBINDPROC_SETDOM:
		ypbind_set_binding(rqstp, transp);
		break;

	default:
		svcerr_noproc(transp);
		break;

	}
}

/*
 * This is a Unix SIGCHILD handler which notices when a broadcaster child
 * process has exited, and retrieves the exit status.  The outputs from this
 * function are broadcast_done, which will be set TRUE, and
 * broadcast_binding_succeeded, which will be set TRUE if the child process
 * exited with a 0 status, and FALSE otherwise.
 */

void
broadcast_proc_exit()
{
	int pid;
	union wait wait_status;
	int termsig, retcode;

	broadcast_done = TRUE;
	pid = 0;

	while (pid != broadcaster_pid) {
		pid = wait3(&wait_status, WNOHANG, NULL);

		if (pid == 0) {
			return;
		} else if (pid == -1) {
			return;
		}
	}

	termsig = wait_status.w_termsig;
	retcode = wait_status.w_retcode;

	if ((termsig == 0) && (retcode == 0)) {
		broadcast_binding_succeeded = TRUE;
	} else {
		broadcast_binding_succeeded = FALSE;
	}
}

/*
 * This returns the current binding for a passed domain.
 */
void
ypbind_get_binding(rqstp, transp)
	struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	char domain_name[YPMAXDOMAIN + 1];
	char *pdomain_name = domain_name;
	char *pname;
	struct ypbind_resp response;
	bool true;
	char outstring[YPMAXDOMAIN + 256];

	if (!svc_getargs(transp, xdr_ypdomain_wrap_string, &pdomain_name) ) {
		svcerr_decode(transp);
		return;
	}

	if ( (current_domain = ypbind_point_to_domain(pdomain_name) ) !=
	    (struct domain *) NULL) {

		/*
		 * Ping the server to make sure it is up.
		 */
		 
		if (current_domain->dom_boundp) {
			ypbind_ping(current_domain);
		}

		/*
		 * Bound or not, return the current state of the binding.
		 */

		if (current_domain->dom_boundp) {
			response.ypbind_status = YPBIND_SUCC_VAL;
			response.ypbind_respbody.ypbind_bindinfo.ypbind_binding_addr =
			    current_domain->dom_serv_addr;
			response.ypbind_respbody.ypbind_bindinfo.ypbind_binding_port = 
			    current_domain->dom_serv_port;
		} else {
			response.ypbind_status = YPBIND_FAIL_VAL;
			response.ypbind_respbody.ypbind_error =
			    YPBIND_ERR_NOSERV;
		}
		    
	} else {
		response.ypbind_status = YPBIND_FAIL_VAL;
		response.ypbind_respbody.ypbind_error = YPBIND_ERR_RESC;
	}

	if (!svc_sendreply(transp, xdr_ypbind_resp, &response) ) {
		fprintf(stderr, "ypbind:  Can't respond to rpc request.\n");
	}

	if (!svc_freeargs(transp, xdr_ypdomain_wrap_string, &pdomain_name) ) {
		fprintf(stderr,
		    "ypbind:  ypbind_get_binding can't free args.\n");
	}

	if ((current_domain) && (!current_domain->dom_boundp) &&
	    (!broadcaster_pid)) {
		/*
		 * The current domain is unbound, and there is no broadcaster 
		 * process active now.  Fork off a child who will yell out on 
		 * the net.  Because of the flavor of request we're making of 
		 * the server, we only expect positive ("I do serve this
		 * domain") responses.
		 */
		broadcast_domain = current_domain;
		broadcast_domain->dom_report_success++;
		pname = current_domain->dom_name;
				
		if ( (broadcaster_pid = fork() ) == 0) {
			(void) clnt_broadcast(YPPROG, YPVERS,
			    YPPROC_DOMAIN_NONACK, xdr_ypdomain_wrap_string,
			    &pname, xdr_int, &true, ypbind_broadcast_ack);
			    
			if (current_domain->dom_boundp) {
				
				/*
				 * Send out a set domain request to our parent
				 */
				ypbind_send_setdom(pname,
				    current_domain->dom_serv_addr,
				    current_domain->dom_serv_port);
				    
				if (current_domain->dom_report_success > 0) {
					sprintf(outstring,
					    "yp: server for domain \"%s\" OK",
					    pname);
					writeit(outstring);
				}
					
				exit(0);
			} else {
				sprintf(outstring,
				    "\
yp: server not responding for domain \"%s\"; still trying",
				    pname);
				writeit(outstring);
				exit(1);
			}

		} else if (broadcaster_pid == -1) {
			fprintf(stderr,
			"ypbind:  broadcaster fork failure.\n");
			broadcaster_pid = 0;
		}
	}
}

static int
writeit(s)
	char *s;
{
	FILE *f;

	if ((f = fopen("/dev/console", "w")) != NULL) {
		fprintf(f, "%s.\n", s);
		fclose(f);
	}
	
}

/*
 * This sends a ypbind "Set domain" message back to our parent.
 */
void
ypbind_send_setdom(dom, addr, port)
	char *dom;
	struct in_addr addr;
	unsigned short int port;
{
	struct ypbind_setdom req;
	struct sockaddr_in myaddr;
	int socket;
	struct timeval timeout;
	struct timeval intertry;
	CLIENT *client;

	strcpy(req.ypsetdom_domain, dom);
	req.ypsetdom_addr = addr;
	req.ypsetdom_port = port;
	get_myaddress(&myaddr);
	myaddr.sin_port = htons(udphandle->xp_port);
	socket = RPC_ANYSOCK;
	timeout.tv_sec = SETDOMTOTTIM;
	intertry.tv_sec = SETDOMINTERTRY;
	timeout.tv_usec = intertry.tv_usec = 0;

	if ((client = clntudp_create (&myaddr, YPBINDPROG, YPBINDVERS,
	    intertry, &socket) ) != NULL) {

		clnt_call(client, YPBINDPROC_SETDOM, xdr_ypbind_setdom,
		    &req, xdr_void, 0, timeout);
		clnt_destroy(client);
		close(socket);
	}
}

/*
 * This sets the internet address and port for the passed domain to the
 * passed values, and marks the domain as supported.
 */
void
ypbind_set_binding(rqstp, transp)
	struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	struct ypbind_setdom req;

	if (!svc_getargs(transp, xdr_ypbind_setdom, &req) ) {
		svcerr_decode(transp);
		return;
	}

	if (!svc_sendreply(transp, xdr_void, 0) ) {
		fprintf(stderr, "ypbind:  Can't reply to rpc call.\n");
	}

	if ( (current_domain = ypbind_point_to_domain(req.ypsetdom_domain) ) !=
	    (struct domain *) NULL) {
		current_domain->dom_serv_addr = req.ypsetdom_addr;
		current_domain->dom_serv_port = req.ypsetdom_port;
		current_domain->dom_boundp = TRUE;
	}
}
/*
 * This returns a pointer to a domain entry.  If no such domain existed on
 * the list previously, an entry will be allocated, initialized, and linked
 * to the list.  Note:  If no memory can be malloc-ed for the domain structure,
 * the functional value will be (struct domain *) NULL.
 */
static struct domain *
ypbind_point_to_domain(pname)
	register char *pname;
{
	register struct domain *pdom;
	
	for (pdom = known_domains; pdom != (struct domain *)NULL;
	    pdom = pdom->dom_pnext) {
		if (!strcmp(pname, pdom->dom_name))
			return (pdom);
	}
	
	/* Not found.  Add it to the list */
	
	if (pdom = (struct domain *)malloc(sizeof (struct domain))) {
		pdom->dom_pnext = known_domains;
		known_domains = pdom;
		strcpy(pdom->dom_name, pname);
		pdom->dom_boundp = FALSE;
		pdom->dom_report_success = -1;
	}
	
	return (pdom);
}

/*
 * This is called by the broadcast rpc routines to process the responses 
 * coming back from the broadcast request. Since the form of the request 
 * which is used in ypbind_broadcast_bind is "respond only in the positive  
 * case", the internet address of the responding server will be picked up 
 * from the saddr parameter, and stuffed into the domain.  The domain's
 * boundp field will be set TRUE.  Because this function returns TRUE, 
 * the first responding server will be the bound server for the domain.
 */
bool
ypbind_broadcast_ack(ptrue, saddr)
	bool *ptrue;
	struct sockaddr_in *saddr;
{
	current_domain->dom_boundp = TRUE;
	current_domain->dom_serv_addr = saddr->sin_addr;
	current_domain->dom_serv_port = saddr->sin_port;
	return(TRUE);
}

/*
 * This checks to see if a server bound to a named domain is still alive and
 * well.  If he's not, boundp in the domain structure is set to FALSE.
 * pdom->boundp may change state.  Note:  Any rpc or resource error, or
 * heavily loaded server machine, or queued-up server, or busy disk, or ....
 * may toggle dom_boundp to FALSE.
 */
void
ypbind_ping(pdom)
       register struct domain *pdom;
{
        u_long port;  /* may be different from pdom->dom_serv_port */
        struct sockaddr_in addr;
        enum clnt_stat clnt_stat;

        addr.sin_addr = pdom->dom_serv_addr;
        addr.sin_family = AF_INET;

        if ((clnt_stat = pmap_rmtcall(&addr, YPPROG, YPVERS, YPPROC_NULL,
            xdr_void, 0, xdr_void, 0, total_timeout, &port)) != RPC_SUCCESS) {
                pdom->dom_boundp = FALSE;
        } else {
                pdom->dom_serv_port = htons((u_short)port);
        }
}

/*
 * Preloads the default domain's domain binding. A domain binding for the
 * local node's default domain will be set up, and a binding to a server
 * may additionally be made.
 */
static void
ypbind_init_default()
{
	char domain[256];
	char *pname = domain;
	int true;
	int binding_tries = 0;

	if (getdomainname(domain, 256) == 0) {
		current_domain = ypbind_point_to_domain(domain);

		if (current_domain == (struct domain *) NULL) {
			abort();
		}
		
		while ( (!current_domain->dom_boundp) &&
		    (binding_tries < BINDING_TRIES) ) {
			(void) clnt_broadcast(YPPROG, YPVERS,
			    YPPROC_DOMAIN_NONACK, xdr_ypdomain_wrap_string,
			    &pname, xdr_int, &true, ypbind_broadcast_ack);
			binding_tries++;
		}
	}
}
