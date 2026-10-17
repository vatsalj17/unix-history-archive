#ifndef lint
static	char sccsid[] = "@(#)yp_bind.c 1.1 85/05/30 Copyr 1985 Sun Micro";
#endif

#include <dbm.h>			/* Pull this in first */
#undef NULL				/* Remove dbm.h's definition of NULL */
extern void dbmclose();			/* Refer to dbm routine not in dbm.h */
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <netdb.h>
#include <sys/socket.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
extern int errno;
extern void sleep();

/*
 * Time parameters when talking to the ypbind and pmap processes
 */

#define YPSLEEPTIME 5			/* Time to sleep between tries */
int _ypsleeptime = YPSLEEPTIME;

#define YPBIND_TIMEOUT 5		/* Total seconds for timeout */
#define YPBIND_INTER_TRY 5		/* Seconds between tries */

static struct timeval bind_intertry = {
	YPBIND_INTER_TRY,		/* Seconds */
	0				/* Microseconds */
	};
static struct timeval bind_timeout = {
	YPBIND_TIMEOUT,			/* Seconds */
	0				/* Microseconds */
	};

/*
 * Time parameters when talking to the ypserv process
 */

#ifdef  DEBUGGING
#define YPTIMEOUT 120			/* Total seconds for timeout */
#define YPINTER_TRY 60			/* Seconds between tries */
#else
#define YPTIMEOUT 20			/* Total seconds for timeout */
#define YPINTER_TRY 5			/* Seconds between tries */
#endif

static struct timeval ypserv_intertry = {
	YPINTER_TRY,			/* Seconds */
	0				/* Microseconds */
	};
struct timeval _ypserv_timeout = {
	YPTIMEOUT,			/* Seconds */
	0				/* Microseconds */
	};

static unsigned long binder_port = 0;	/* Local port for the ypbinder */
static struct in_addr my_addr;		/* Local internet addr */
static struct dom_binding *bound_domains = NULL; /* Ptr to list of bound
					 *   domains */
static char default_domain[256];

/*
 * Attempts to locate a yellow pages server which serves a passed domain.  If
 * one is found, an entry is created on the static list of domain-server pairs
 * pointed to by cell bound_domains, a udp path to the server is created and
 * the function returns 0.  Otherwise, the function returns a defined errorcode
 * YPERR_xxxx.
 */
int
_yp_dobind(domain, binding)
	char *domain;
	struct dom_binding **binding;	/* if result == 0, ptr to dom_binding */
{
	struct in_addr server_addr;	/* Internet addr of yp server */
	struct dom_binding *pdomb;	/* Ptr to new domain binding */
	int sokt;			/* Socket to talk to pmapper, ypbinder */
	CLIENT *client;			/* Client for ditto */
	enum clnt_stat clnt_stat;	/* Value of rpc calls */
	struct sockaddr_in pmapper;	/* To talk with pmapper */
	struct sockaddr_in ypbinder;	/* To talk with ypbinder */
	struct sockaddr_in dummy;	/* To get a port bound to socket */
	struct sockaddr_in local_name;	/* To check our socket binding */
	int local_name_len = sizeof(struct sockaddr_in); /* Parameter for
					 *   getsockname */
	bool rebind = FALSE;
	bool oldport;			/* TRUE if we're using a cached
					 *   port for the binder. */
	struct pmap portmap;		/* For pmapper interface */
	char *pdomain = domain;		/* For xdr interface */
	struct ypbind_resp ypbind_resp; /* Response from local ypbinder */
	char buf[BUFSIZ];
	int status;

	if ( (domain == NULL) ||(strlen(domain) == 0) ) { /* Check args */
		return(YPERR_BADARGS);
	}
	
	/*
	 * If the domain has already been bound, make sure that the socket
	 * hasn't been closed under us.  If it has, unbind and continue
	 * to try to bind again.
	 */

	for (pdomb = bound_domains; pdomb != NULL; pdomb = pdomb->dom_pnext) {
		
		if (strcmp(domain, pdomb->dom_domain) == 0) {

			if (getsockname(pdomb->dom_socket, (struct sockaddr *)
			    &local_name, &local_name_len) == 0) {

				if (local_name.sin_family != AF_INET) {
					rebind = TRUE;
				}

				if (pdomb->dom_local_port !=
				    local_name.sin_port) {
					rebind = TRUE;
				}
				
			} else {
				rebind = TRUE;
			}

			if (rebind) {
				
				/*
				 * Don't let yp_unbind close a socket which
				 *   may be open and in use by an upper level.
				 */
				 
				pdomb->dom_socket = -1;
				yp_unbind(domain);
				pdomb = (struct dom_binding *) NULL;
				break;
			} else {
				*binding = pdomb;
				return(0);
			}
		}
	}

	/*
	 * Use the ypbinder's port if we already have it; otherwise, go through
	 * a horrible song and dance to get it.  There is a goto and a goto
	 * target here.
	 */

no_binder_port:

	if (binder_port) {
		oldport = TRUE;
		goto do_bind_request;
	} else {
		oldport = FALSE;
	}

	/*
	 * See whether the portmapper is up.  If the connect succeeds, the
	 * portmapper is dead.
	 */
	get_myaddress(&pmapper);
	my_addr = pmapper.sin_addr;	/* Remember this for other uses */
	sokt =  socket(AF_INET, SOCK_STREAM, 0); /* Throw-away socket */
	
	if (sokt == -1) {
		return(YPERR_RESRC);
	}

	if ((status = connect(sokt, (struct sockaddr *) &pmapper,
	    sizeof(struct sockaddr_in))) < 0) {
		close(sokt);
		return(YPERR_PMAP);
	}
	
	close(sokt);

	/*
	 * Ask the portmapper for ypbinder's port.
	 */

	portmap.pm_prog = YPBINDPROG;
	portmap.pm_vers = YPBINDVERS;
	portmap.pm_prot = IPPROTO_UDP;
	portmap.pm_port = 0;		/* Don't care */

	sokt = RPC_ANYSOCK;

	if ((client = clntudp_create(&pmapper, PMAPPROG, PMAPVERS,
	    bind_intertry, &sokt))  == NULL) {
		return(YPERR_RPC);
	}

	clnt_stat = (enum clnt_stat) clnt_call(client, PMAPPROC_GETPORT,
	    xdr_pmap, &portmap, xdr_u_long, &binder_port, bind_timeout);
	clnt_destroy(client);
	close(sokt);
	
	if (clnt_stat  == RPC_SUCCESS) {

		if (binder_port == 0) {
			/* There's no binder. Give up. */
			return(YPERR_YPBIND);
		};

	} else {
		/* Try to talk to the port mapper again. */
		sleep(YPSLEEPTIME);
		goto no_binder_port;
	}

	/*
	 * At this point, we are unbound, we have the binder's port, but we
	 * don't know whether he's alive.  Ask ypbinder for the internet
	 * address of a server for the passed domain, after checking to see if
	 * ypbind's alive first.  Instead of connecting to the binder's socket,
	 * we'll try to bind it, which is faster and doesn't keep the socket
	 * open in timed wait state.  There is a goto target here.
	 */

do_bind_request:	

	ypbinder.sin_addr.S_un.S_addr = 0;
	ypbinder.sin_family = AF_INET;
	ypbinder.sin_port = htons(binder_port);
	sokt =  socket(AF_INET, SOCK_DGRAM, 0); /* Throw-away socket */
	
	if (sokt == -1) {
		return(YPERR_RESRC);
	}

	errno = 0;
	status = bind(sokt, (struct sockaddr *) &ypbinder,
	    sizeof(struct sockaddr_in));
	close(sokt);
	    
	if ( (status == 0) || ((status == -1) && (errno != EADDRINUSE) ) ) {
		binder_port = 0;
		
		if (oldport) {
			/*
			 * If we're using a cached port, assume that the
			 * binder has been killed and restarted, and try to
			 * get a fresh port for him.
			 */
			goto no_binder_port;
		} else {
			/*
			 * We are using what the portmap believes to be
			 * current information.  Assume that the binder
			 * is dead.  Give up.
			 */
			return(YPERR_YPBIND);
		}
	}


	/*
	 * At this point, we think we know how to talk to the binder, and
	 * the binder is apparently alive.  Until we succeed in binding the
	 * domain, we will try forever.  The only things except success which
	 * will let us return after this point are local resource allocation
	 * failures, and backtrack paths during which we lose the ability to
	 * talk to the port mapper or the binder.
	 */

	for (;;) {
		ypbinder.sin_addr = my_addr;
		sokt = RPC_ANYSOCK;

		if ((client = clntudp_create(&ypbinder, YPBINDPROG, YPBINDVERS,
	 	   bind_intertry, &sokt))  == NULL) {
			return(YPERR_RPC);
		}

		clnt_stat = (enum clnt_stat) clnt_call(client, YPBINDPROC_DOMAIN,
		    xdr_ypdomain_wrap_string, &pdomain, xdr_ypbind_resp,
		    &ypbind_resp, bind_timeout);
		clnt_destroy(client);
		close(sokt);
	    
		if (clnt_stat == RPC_SUCCESS) {
			
			if (ypbind_resp.ypbind_status == YPBIND_SUCC_VAL) {
				/* Binding successfully returned from ypbind */
				break;	
			} else {
				sleep(YPSLEEPTIME);
			}
			
		} else {
			sleep(YPSLEEPTIME);
			binder_port = 0;
			goto no_binder_port;
		}
	}
	
	/*
	 * Allocate some memory for a domain binding, initialize it, and
	 * return a pointer to it.
	 */
	pdomb = (struct dom_binding *) NULL;
	 
	if ((pdomb = (struct dom_binding *) malloc(sizeof(struct dom_binding)))
		== NULL) {
		fprintf(stderr, "_yp_dobind:  malloc failure.\n");
		return(YPERR_RESRC);
	}

	pdomb->dom_server_addr.sin_addr =
	    ypbind_resp.ypbind_respbody.ypbind_bindinfo.ypbind_binding_addr;
	pdomb->dom_server_addr.sin_family = AF_INET;
	pdomb->dom_server_addr.sin_port =
	    ypbind_resp.ypbind_respbody.ypbind_bindinfo.ypbind_binding_port;
	pdomb->dom_server_port =
	    ypbind_resp.ypbind_respbody.ypbind_bindinfo.ypbind_binding_port;
	pdomb->dom_socket = RPC_ANYSOCK;

	/*
	 * Open up a udp path to the server, which will remain active globally.
	 */

	if ((pdomb->dom_client = clntudp_create(&(pdomb->dom_server_addr),
	    YPPROG, YPVERS, ypserv_intertry, &(pdomb->dom_socket)))  == NULL) {
		free(pdomb);
		return(YPERR_RPC);
	}

	/*
	 * Bind the socket to a bogus address so a port gets allocated for
	 * the socket, but so that sendto will still work.
	 */

	dummy.sin_family = AF_INET;
	dummy.sin_addr.s_addr = 0;
	dummy.sin_port = 0;
	
	if (bind (pdomb->dom_socket, (struct sockaddr *) &dummy,
	    sizeof(dummy) ) != 0) {
		free(pdomb);
		return(YPERR_YPERR);
	}

	/* Remember the bound port number  */
	
	if (getsockname(pdomb->dom_socket, (struct sockaddr *) &local_name,
	    &local_name_len) == 0) {
		pdomb->dom_local_port = local_name.sin_port;
	} else {
		free(pdomb);
		return(YPERR_YPERR);
	}

	strcpy(pdomb->dom_domain, domain);	/* Remember the domain name */
	pdomb->dom_pnext = bound_domains;	/* Link this to the list as */
	bound_domains = pdomb;			/* ... the head entry */
	*binding = pdomb;			/* Return ptr to the binding
						 *   entry */
	return(0);				/* This is the go path */
}

/*
 * This is a "wrapper" function for _yp_dobind for vanilla user-level
 * functions which neither know nor care about struct dom_bindings.
 */
int
yp_bind(domain)
	char *domain;
{
	 
	struct dom_binding *binding;
	
	return(_yp_dobind(domain, &binding) );
}

/*
 * Attempts to find a dom_binding in the list at bound_domains having the
 * domain name field equal to the passed domain name, and removes it if found.
 * The domain-server binding will not exist after the call to this function.
 * All resources associated with the binding will be freed.
 */
void
yp_unbind (domain)
	char *domain;
{
	struct dom_binding *pdomb;
	struct dom_binding *ptrail;
	

	if ( (domain == NULL) ||(strlen(domain) == 0) ) {
		return;
	}
	
	for (pdomb = bound_domains; pdomb != NULL;
	    ptrail = pdomb, pdomb = pdomb->dom_pnext) {
		
		if (strcmp(domain, pdomb->dom_domain) == 0) {
			clnt_destroy(pdomb->dom_client);
			close(pdomb->dom_socket);

			if (pdomb == bound_domains) {
				bound_domains = pdomb->dom_pnext;
			} else {
				ptrail->dom_pnext = pdomb->dom_pnext;
			}

			free(pdomb);
			break;	
		}
		
	}


}

/*
 * This is a wrapper for the system call getdomainname which returns a
 * ypclnt.h error code in the failure case.  It also checks to see that
 * the domain name is non-null, knowing that the null string is going to
 * get rejected elsewhere in the yp client package.
 */
int
yp_get_default_domain (domain)
	char **domain;
{
	if (getdomainname(default_domain, 256) == 0) {

		if (strlen(default_domain) > 0) {
			*domain = default_domain;
			return(0);
		} else {
			return(YPERR_NODOM);
		}
		
	} else {
		return(YPERR_YPERR);
	}
}
