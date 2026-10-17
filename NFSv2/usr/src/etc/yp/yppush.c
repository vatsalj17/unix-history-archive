#ifndef lint
static	char sccsid[] = "@(#)yppush.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * This is a user command which issues a "push", "pull", or "poll" command to a 
 * yellowpages server (ypserv), dependent upon the name it is installed and
 * run under.  The syntax is:
 *
 *	yppush [-h <host>] [-d <domainname>] mapname
 * or
 *	yppull [-h <host>] [-d <domainname>] mapname
 * or
 *	yppoll [-h <host>] [-d <domainname>] mapname
 *
 * where host may be either a name or an internet address of form ww.xx.yy.zz
 *
 * If the host is ommitted, the local host will be used.  If host is specified
 * as an internet address, no yp services need to be locally available.
 */
#include <dbm.h>
#ifdef NULL
#undef NULL
#endif
#define NULL 0
#include <stdio.h>
#include <sys/time.h>
#include <ctype.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>

#ifdef NULL
#undef NULL
#endif
#define NULL 0

#define TIMEOUT 30			/* Total seconds for timeout */
#define INTER_TRY 10			/* Seconds between tries */

bool dopush = FALSE;
bool dopull = FALSE;
bool dopoll = FALSE;
struct ypresponse poll_resp;
char *pusage;
char *domain = NULL;
char default_domain_name[YPMAXDOMAIN];
char *map = NULL;
char *host = NULL;
char default_host_name[256];
struct in_addr host_addr;
struct timeval udp_intertry = {
	INTER_TRY,			/* Seconds */
	0				/* Microseconds */
	};
struct timeval udp_timeout = {
	TIMEOUT,			/* Seconds */
	0				/* Microseconds */
	};

char err_usage_push[] =
"Usage:\n\
	yppush [-h <host>] [-d <domainname>] mapname\n\n\
where host may be either a name or an internet \n\
address of form ww.xx.yy.zz\n";
char err_usage_pull[] =
"Usage:\n\
	yppull [-h <host>] [-d <domainname>] mapname\n\n\
where host may be either a name or an internet \n\
address of form ww.xx.yy.zz\n";
char err_usage_poll[] =
"Usage:\n\
	yppoll [-h <host>] [-d <domainname>] mapname\n\n\
where host may be either a name or an internet \n\
address of form ww.xx.yy.zz\n";
char err_cant_find_server_addr[] =
	"Sorry, I can't get an address for host %s from the yellow pages.\n";
char err_cant_decipher_server_addr[] =
     "Sorry, I got a garbage address for host %s back from the yellow pages.\n";
char err_bad_args[] =
	"Sorry, the %s argument is bad.\n";
char err_cant_get_kname[] =
	"Sorry, can't get %s back from system call.\n";
char err_null_kname[] =
	"Sorry, the %s hasn't been set on this machine.\n";
char err_bad_hostname[] = "hostname";
char err_bad_mapname[] = "mapname";
char err_bad_domainname[] = "domainname";
char err_cant_bind[] =
	"Sorry, I can't make use of the yellow pages.  I give  up.\n";
char err_udp_failure[] =
	"Sorry, I can't set up a udp connection to ypserv on host %s.\n";
char err_rpc_failure[] =
	"Sorry, I couldn't send my rpc message to ypserv on host %s.\n";
char err_bad_resp[] =
	"Sorry, ill-formed response returned by ypserv on host %s.\n";

extern char *rindex();
void get_command_line_args();
void get_host_addr();
void send_message();
void report_poll_results();

/*
 * Funny external reference to inet_addr to make the reference agree with the
 * code, not the documentation.  Should be:
 * extern struct in_addr inet_addr();
 * according to the documentation, but that's not what the code does.
 */
extern u_long inet_addr();

/*
 * This is the mainline for the yppush process.  It pulls whatever arguments
 * have been passed from the command line, and uses defaults for the rest.
 */

void
main (argc, argv)
	int argc;
	char **argv;
	
{
	struct sockaddr_in myaddr;

	get_command_line_args(argc, argv);

	if (!domain) {
		
		if (!getdomainname(default_domain_name, YPMAXDOMAIN) ) {
			domain = default_domain_name;
		} else {
			fprintf(stderr, err_cant_get_kname, err_bad_domainname);
			exit(1);
		}

		if (strlen(domain) == 0) {
			fprintf(stderr, err_null_kname, err_bad_domainname);
			exit(1);
		}
	}
	
	if (!host) {
		
		if (! gethostname(default_host_name, 256)) {
			host = default_host_name;
		} else {
			fprintf(stderr, err_cant_get_kname, err_bad_hostname);
			exit(1);
		}

		get_myaddress(&myaddr);
		host_addr = myaddr.sin_addr;
	} else {	
		get_host_addr();
	}
	
	send_message();
	exit(0);
}

/*
 * This does the command line argument processing.
 */
void
get_command_line_args(argc, argv)
	int argc;
	char **argv;
	
{
	char *progname;

	progname = rindex(*argv, '/');

	if (progname) {
		progname++;
	} else {
		progname = *argv;
	}
	
	if (!strcmp(progname, "yppush") ) {
		dopush = TRUE;
		pusage = err_usage_push;
	} else if (!strcmp(progname, "yppull") ) {
		dopull = TRUE;
		pusage = err_usage_pull;
	} else {
		dopoll = TRUE;
		pusage = err_usage_poll;
	}

	argv++;
	
	while (--argc > 1) {

		if ( (*argv)[0] == '-') {

			switch ((*argv)[1]) {

			case 'h': {

				if (argc > 1) {
					argv++;
					argc--;
					host = *argv;
					argv++;

					if (strlen(host) > 256) {
						fprintf(stderr, err_bad_args,
						    err_bad_hostname);
						exit(1);
					}
					
				} else {
					fprintf(stderr, pusage);
					exit(1);
				}
				
				break;
			}
				
			case 'd': {

				if (argc > 1) {
					argv++;
					argc--;
					domain = *argv;
					argv++;

					if (strlen(domain) > YPMAXDOMAIN) {
						fprintf(stderr, err_bad_args,
						    err_bad_domainname);
						exit(1);
					}
					
				} else {
					fprintf(stderr, pusage);
					exit(1);
				}
				
				break;
			}
				
			default: {
				fprintf(stderr, pusage);
				exit(1);
			}
			
			}
			
		} else {
			fprintf(stderr, pusage);
			exit(1);
		}
	}

	if (argc == 1) {
		
		if ( (*argv)[0] == '-') {
			fprintf(stderr, pusage);
			exit(1);
		}
		
		map = *argv;

		if (strlen(map) > YPMAXMAP) {
			fprintf(stderr, err_bad_args,
			    err_bad_mapname);
			exit(1);
		}

	} else {
		fprintf(stderr, pusage);
		exit(1);
	}
}

/*
 * This tries to contact the yellow pages (any server) and tries to get a
 * match on the host name.  It then calls the standard network function
 * inet_addr to turn the returned string (equivalent to that which is in
 * /etc/hosts) to an internet address.
 */
 
void
get_host_addr()
{
	char *ascii_addr;
	int addr_len;
	struct in_addr tempaddr;
	
	if (isdigit(*host) ) {
		tempaddr.s_addr = inet_addr(host);

		if ((int) tempaddr.s_addr != -1) {
			host_addr = tempaddr;
			return;
		}
	}
	
	if (!yp_bind("yp_private") ) {
		
		if (!yp_match ("yp_private", "hosts.byname", host, strlen(host),
		    &ascii_addr, &addr_len) ) {
			tempaddr.s_addr = inet_addr(ascii_addr);

			if ((int) tempaddr.s_addr != -1) {
				host_addr = tempaddr;
			} else {
				fprintf(stderr, err_cant_decipher_server_addr,
				    host);
				exit(1);
			}

		} else {
			fprintf(stderr, err_cant_find_server_addr, host);
			exit(1);
		}
		
	} else {
		fprintf(stderr, err_cant_bind);
		exit(1);
	}
}

/*
 * This takes the internet address of the yellow pages host of interest,
 * and fires off the "push", "pull", or "poll" message to the ypserv process.
 */
 
void
send_message()
{
	struct dom_binding domb;
	struct yprequest req;
	struct ypresponse resp;
	enum clnt_stat clnt_stat;

	domb.dom_server_addr.sin_addr = host_addr;
	domb.dom_server_addr.sin_family = AF_INET;
	domb.dom_server_addr.sin_port = 0;
	domb.dom_server_port = 0;
	domb.dom_socket = RPC_ANYSOCK;

	/*
	 * Open up a udp path to the server
	 */

	if ((domb.dom_client = clntudp_create(&(domb.dom_server_addr),
	    YPPROG, YPVERS, udp_intertry, &(domb.dom_socket)))  == NULL) {
		fprintf(stderr, err_udp_failure, host);
		exit(1);
	}

	/*
	 * Check to see if the mapname is one which is in the domain's
	 * ypmaps.
	 */
	req.yp_reqtype = YPMATCH_REQTYPE;
	req.ypmatch_req_domain = domain;
	req.ypmatch_req_map = "ypmaps";
	req.ypmatch_req_keyptr = map;
	req.ypmatch_req_keysize = strlen(map);

	resp.ypmatch_resp_valptr = NULL;
	resp.ypmatch_resp_valsize = 0;
	
	if( (clnt_stat = (enum clnt_stat) clnt_call(domb.dom_client,
	    YPPROC_MATCH, xdr_yprequest, &req, xdr_ypresponse, &resp,
	    udp_timeout) ) != RPC_SUCCESS) {
		fprintf(stderr, err_rpc_failure, host);
		exit(1);
	}

	if (resp.ypmatch_resp_status != YP_TRUE) {

		switch (resp.ypmatch_resp_status) {
		case YP_NOKEY: {
			fprintf(stderr,
		"Map %s's name is not in ypmaps in domain %s on host %s.\n",
			    map, domain, host);
			break;
		}

		default: {
			fprintf(stderr,
		"YP error number %d trying to check map name.\n",
			    resp.ypmatch_resp_status);
			break;
		}

		}

		exit(1);
	}

	/*
	 * Load up the message structure and fire it off.
	 */

	if (dopush) {
		req.yp_reqtype = YPPUSH_REQTYPE;
		req.yppush_req_domain = domain;
		req.yppush_req_map = map;
	
		if( (clnt_stat = (enum clnt_stat) clnt_call(domb.dom_client,
		    YPPROC_PUSH, xdr_yprequest, &req, xdr_void, 0,
		    udp_timeout) ) != RPC_SUCCESS) {
			fprintf(stderr, err_rpc_failure, host);
			exit(1);
		}

	} else if (dopull) {
		req.yp_reqtype = YPPULL_REQTYPE;
		req.yppull_req_domain = domain;
		req.yppull_req_map = map;
	
		if( (clnt_stat = (enum clnt_stat) clnt_call(domb.dom_client,
		    YPPROC_PULL, xdr_yprequest, &req, xdr_void, 0,
		    udp_timeout) ) != RPC_SUCCESS) {
			fprintf(stderr, err_rpc_failure, host);
			exit(1);
		}

	} else {
		req.yp_reqtype = YPPOLL_REQTYPE;
		req.yppoll_req_domain = domain;
		req.yppoll_req_map = map;
	
		if( (clnt_stat = (enum clnt_stat) clnt_call(domb.dom_client,
		    YPPROC_POLL, xdr_yprequest, &req, xdr_ypresponse, &poll_resp,
		    udp_timeout) ) != RPC_SUCCESS) {
			fprintf(stderr, err_rpc_failure, host);
			exit(1);
		}
		
		report_poll_results(&poll_resp, domain, map);
	}
}

/*
 * This dumps out a series of messages to stdout summarizing the results of
 * the poll.
 */

void
report_poll_results(resp, dom, map)
	struct ypresponse *resp;
	char *dom;
	char *map;
	
{
	if (resp->yp_resptype != YPPOLL_RESPTYPE) {
		fprintf(stderr, err_bad_resp, host);
	}

	if (!strcmp(resp->yppoll_resp_domain, dom) ) {
		printf("Domain %s is supported.\n", dom);

		if (!strcmp(resp->yppoll_resp_map, map) ) {

			if (resp->yppoll_resp_ordernum != 0) {
				printf("Map %s has order number %d.\n",
				    map, resp->yppoll_resp_ordernum);

				if (strcmp(resp->yppoll_resp_owner, "") ) {
					printf("The master server is %s.\n",
					    resp->yppoll_resp_owner);
				} else {
					printf("Unknown master server.\n");
				}
				
			} else {
				printf("Map %s is not supported.\n", map);
			}
			
		} else {
			printf("Map %s does not exist at %s.\n", map, host);
		}
		
	} else {
		printf("Domain %s is not supported.\n", dom);
	}
}
