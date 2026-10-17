#ifndef lint
static  char sccsid[] = "@(#)ypwhich.c 1.1 85/05/31 Copyr 1985 Sun Micro";
#endif

/*
 * This is a user command which tells which yp server is being used by
 * a given machine, or which yp server is the master for a named map, or
 * prints a list of all maps in the domain, and tells which yp server is
 * master of each of them.
 *
 * Usage is:
 *	ypwhich [-d domain] [-t] [-m [name] | host]
 *
 *  where:  the -d switch can be used to specify a domain other than the
 * default domain.  -m with a name tells the master of that map, -m with no
 * name gives a list of all maps in the domain and their associated
 * masters.  If name is specified it may be either a mapname, or a nickname
 * which will be translated into a mapname according to the translation
 * table at transtable.  The  -t switch inhibits this translation.  If the
 * -m option is used, ypwhich will act like a vanilla yp client, and will
 * not attempt to choose a particular yp server.  On the other hand, if no
 * -m switch is used, ypwhich will talk directly to the yp bind process on
 * the named host, or to the local ypbind process if no host name is
 * specified.
 */

#include <dbm.h>
#undef NULL
#include <stdio.h>
#include <rpc/rpc.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

void get_command_line_args();
void getdomain();
void getlochost();
void getrmthost();
void call_binder();
void get_map_master();
void dump_ypmaps();

#define TIMEOUT 30			/* Total seconds for timeout */
#define INTER_TRY 10			/* Seconds between tries */

int translate = TRUE;
char *domain = NULL;
char default_domain_name[YPMAXDOMAIN];
char *host = NULL;
char default_host_name[256];
struct in_addr host_addr;
bool get_master = FALSE;
bool get_server = FALSE;
char *map = NULL;
struct timeval udp_intertry = {
	INTER_TRY,			/* Seconds */
	0				/* Microseconds */
	};
struct timeval udp_timeout = {
	TIMEOUT,			/* Seconds */
	0				/* Microseconds */
	};
char *transtable[] = {
	"passwd", "passwd.byname",
	"group", "group.byname",
	"networks", "networks.byaddr",
	"hosts", "hosts.byaddr",
	"protocols","protocols.bynumber",
	"services","services.byname",
	NULL
};
char err_usage[] =
"Usage:\n\
	ypwhich [-d domain] [-t] [-m [name] | host]\n";
char err_bad_args[] =
	"ypwhich:  %s argument is bad.\n";
char err_cant_get_kname[] =
	"ypwhich:  can't get %s back from system call.\n";
char err_null_kname[] =
	"ypwhich:  the %s hasn't been set on this machine.\n";
char err_bad_mapname[] = "mapname";
char err_bad_domainname[] = "domainname";
char err_bad_hostname[] = "hostname";
char err_cant_bind[] =
	"ypwhich:  can't bind to yp server for domain %s.  Reason:  %s.\n";
char err_first_failed[] =
	"ypwhich:  can't get first record from yp.  Reason:  %s.\n";
char err_next_failed[] =
	"ypwhich:  can't get next record from yp.  Reason:  %s.\n";
char err_udp_failure[] =
	"ypwhich:  can't set up a udp connection to ypserv on host %s.\n";
char err_rpc_failure[] =
	"ypwhich:  couldn't send rpc message to ypserv on host %s.\n";

/*
 * This is the main line for the ypwhich process.
 */
main(argc, argv)
	char **argv;
{
	int addr;
	int err;
	struct ypbind_resp response;
	int i;

	get_command_line_args(argc, argv);

	if (!domain) {
		getdomain();
	}
	
	if (get_server) {
		
		if (!host) {
			getlochost();
		} else {
			getrmthost();
		}

		call_binder();
	} else {
		
		if (map) {
	
			if (translate) {
						
			    for (i = 0; transtable[i]; i+=2)
					    
				 if (strcmp(map,
				     transtable[i]) == 0) {
					map = transtable[i+1];
				}
			}
			
			get_map_master();
		} else {
			dump_ypmaps();
		}
	}

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
	struct hostent *hp;
	
	argv++;

	if (argc == 1) {
		get_server = TRUE;
		return;
	}
	
	while (--argc) {

		if ( (*argv)[0] == '-') {

			switch ((*argv)[1]) {

			case 't': {
				translate = FALSE;
				argv++;
				break;
			}

			case 'm': {
				get_master = TRUE;
				argv++;
				
				if (argc > 1) {
					
					if ( (*(argv))[0] == '-') {
						break;
					}
					
					argc--;
					map = *argv;
					argv++;

					if (strlen(map) > YPMAXMAP) {
						fprintf(stderr, err_bad_args,
						    err_bad_mapname);
						exit(1);
					}
					
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
					fprintf(stderr, err_usage);
					exit(1);
				}
				
				break;
			}
				
			default: {
				fprintf(stderr, err_usage);
				exit(1);
			}
			
			}
			
		} else {
			
			if (get_server) {
				fprintf(stderr, err_usage);
				exit(1);
			}
		
			get_server = TRUE;
			host = *argv;
			argv++;
			
			if (strlen(host) > 256) {
				fprintf(stderr, err_bad_args, err_bad_hostname);
				exit(1);
			}
		}
	}

	if (get_master && get_server) {
		fprintf(stderr, err_usage);
		exit(1);
	}
	
	if (!get_master && !get_server) {
		get_server = TRUE;
	} 
}

/*
 * This gets the local default domainname, and makes sure that it's set
 * to something reasonable.  domain is set here.
 */
void
getdomain()		
{
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

/*
 * This gets the local hostname back from the kernel, and comes up with an
 * address for the local node without using the yp.  host_addr is set here.
 */
void
getlochost()
{
	struct sockaddr_in myaddr;

	if (! gethostname(default_host_name, 256)) {
		host = default_host_name;
	} else {
		fprintf(stderr, err_cant_get_kname, err_bad_hostname);
		exit(1);
	}

	get_myaddress(&myaddr);
	host_addr = myaddr.sin_addr;
}

/*
 * This gets an address for some named node by calling the standard library
 * routine gethostbyname.  host_addr is set here.
 */
void
getrmthost()
{		
	struct hostent *hp;
	
	hp = gethostbyname(host);
		
	if (hp == NULL) {
	    	fprintf(stderr, "ypwhich: can't find %s\n", host);
		exit(1);
	}
	
	host_addr.s_addr = *(u_long *)hp->h_addr;
}

/*
 * This sends a message to the ypbind process on the node with address held
 * in host_addr, and then translates the returned server address to a name.
 * If the returned address is the same as the local address as returned by
 * get_myaddress, the name is that retrieved from the kernel.  If it's any
 * other address (including another ip address for the local machine), we'll
 * get a name by using the standard library routine (which calls the yp).
 */
void
call_binder()
{
	struct hostent *hp;
	struct sockaddr_in query;
	CLIENT *client;
	int sock = RPC_ANYSOCK;
	enum clnt_stat rpc_stat;
	struct ypbind_resp response;
	struct in_addr server;
	
	query.sin_family = AF_INET;
	query.sin_port = 0;
	query.sin_addr = host_addr;
	
	if ((client =
	    clntudp_create(&query, YPBINDPROG, YPBINDVERS, udp_intertry,
	    &sock)) == NULL) {
		fprintf(stderr, err_udp_failure, host);
		exit(1);
	}

	rpc_stat =
	    clnt_call(client, YPBINDPROC_DOMAIN, xdr_ypdomain_wrap_string,
	    &domain, xdr_ypbind_resp, &response, udp_timeout);
	    
	if (rpc_stat != RPC_SUCCESS) {
		fprintf(stderr, err_rpc_failure, host);
		exit(1);
	}

	if (response.ypbind_status != YPBIND_SUCC_VAL) {
		fprintf(stderr, "ypbinder of %s didn't return a value\n",host);
		exit(1);
	}

	server = response.ypbind_respbody.ypbind_bindinfo.ypbind_binding_addr;
	getlochost();		/* This resets host, host_addr, and my_addr */
	
	if (server.s_addr == host_addr.s_addr) {
		printf(host);
		printf("\n");
	} else {
		hp = gethostbyaddr(&server, sizeof(server), AF_INET);
	
		if (hp == NULL) {
			printf("0x%x\n", server);
		} else {
			printf("%s\n", hp->h_name);
		}
	}
}

/*
 * This does a yp "match" operation within the map "ypmaps" in the domain
 * at global "domain" for the map at global "map".  
 */
void
get_map_master()
{
	int err;
	char *master;
	int len;
     
	err = yp_match(domain, "ypmaps", map, strlen(map), &master, &len);
	
	if (err) {
		
		if (err == YPERR_KEY) {
			fprintf(stderr,
			"ypwhich:  mapname %s is not in ypmaps in domain %s.\n",
			    map, domain);
		} else {
			fprintf(stderr,
			    "ypwhich:  yp_match failed.  Reason: %s.\n",
		    	    yperr_string(err) );
		}
		
	} else {
		printf(master);
	}
}


/*
 * This enumerates the entries within map "ypmaps" in the domain at global 
 * "domain", and prints them out key and value per single line.
 */
void
dump_ypmaps()
{
	char *key;
	int keylen;
	char *outkey;
	int outkeylen;
	char *val;
	int vallen;
	int err;
	char *scan;
	
	if (err = yp_first(domain, "ypmaps", &outkey, &outkeylen, &val,
	    &vallen) ) {

		if (err == YPERR_NOMORE) {
			fprintf(stderr, "ypwhich:  map \"ypmaps\" is empty.\n");
		} else {
			fprintf(stderr, err_first_failed,
			    yperr_string(err) );
		}

		exit(1);
	}

	while (TRUE) {

		for (scan = outkey; *scan != NULL; scan++) {

			if (*scan == '\n') {
				*scan = ' ';  
			}
		}

		printf(outkey);
		printf(val);
		free(val);
		key = outkey;
		keylen = outkeylen;
		
		if (err = yp_next(domain, "ypmaps", key, keylen, &outkey,
		    &outkeylen, &val, &vallen) ) {

			if (err == YPERR_NOMORE) {
				break;
			} else {
				fprintf(stderr, err_next_failed,
				    yperr_string(err) );
				exit(1);
			}
		}

		free(key);
	}
}
