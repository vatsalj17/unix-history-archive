#ifndef lint
static	char sccsid[] = "@(#)ypcat.c 1.1 85/05/31 Copyr 1984 Sun Micro";
#endif

/*
 * This is a user command which dumps each entry in a yp data base.  It gets
 * the stuff using the normal ypclnt package; the user doesn't get to choose
 * which server gives him the input.  Usage is:
 *	ypcat [-a] [-k] [-d domain] [-t] map
 * where the -a switch will dump all records in the map, including the yp
 * private symbols, the -k switch will dump keys followed by a single blank
 * space before the value, and the -d switch can be used to specify a domain
 * other than the default domain.

 * Normally, passwd gets converted to passwd.byname, and similarly
 * for the other standard files.  -t inhibits this translation
 */
#include <dbm.h>
#ifdef NULL
#undef NULL
#endif
#define NULL 0
#include <stdio.h>
#include <rpc/rpc.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>

int dumpkeys = FALSE;
int translate = TRUE;
char *domain = NULL;
char default_domain_name[YPMAXDOMAIN];
char *map = NULL;
int (*getfirst) () = yp_first;
int (*getnext) () = yp_next;
char nullstring[] = "";
char err_usage[] =
"Usage:\n\
	ypcat [-a] [-k] [-d domainname] [-t] mapname\n";
char err_bad_args[] =
	"ypcat:  %s argument is bad.\n";
char err_cant_get_kname[] =
	"ypcat:  can't get %s back from system call.\n";
char err_null_kname[] =
	"ypcat:  the %s hasn't been set on this machine.\n";
char err_bad_mapname[] = "mapname";
char err_bad_domainname[] = "domainname";
char err_cant_bind[] =
	"ypcat:  can't bind to yp server for domain %s.  Reason:  %s.\n";
char err_first_failed[] =
	"ypcat:  can't get first record from yp.  Reason:  %s.\n";
char err_next_failed[] =
	"ypcat:  can't get next record from yp.  Reason:  %s.\n";

void get_command_line_args();
char *ypclnterr2string();

char *transtable[] = {
	"passwd", "passwd.byname",
	"group", "group.byname",
	"networks", "networks.byaddr",
	"hosts", "hosts.byaddr",
	"protocols","protocols.bynumber",
	"services","services.byname",
	NULL
};

/*
 * Funny external reference to inet_addr to make the reference agree with the
 * code, not the documentation.  Should be:
 * extern struct in_addr inet_addr();
 * according to the documentation, but that's not what the code does.
 */
extern u_long inet_addr();

/*
 * This is the mainline for the ypcat process.  It pulls whatever arguments
 * have been passed from the command line, and uses defaults for the rest.
 */

void
main (argc, argv)
	int argc;
	char **argv;
	
{
	char *key;
	int keylen;
	char *outkey;
	int outkeylen;
	char *val;
	int vallen;
	int err;
	char *scan;
	int i;
	
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

	if (err = yp_bind(domain) ) {
		fprintf(stderr, err_cant_bind, domain,
		    ypclnterr2string(err) );
		exit(1);
	}

	key = nullstring;
	keylen = 0;
	val = nullstring;
	vallen = 0;
	
	if (translate) {
		for (i = 0; transtable[i]; i+=2)
			if (strcmp(map, transtable[i]) == 0) {
			    map = transtable[i+1];
			    break;
			}
	}

	if (err = getfirst(domain, map, &outkey, &outkeylen, &val, &vallen) ) {

		if (err == YPERR_NOMORE) {
			exit(0);
		} else {
			fprintf(stderr, err_first_failed,
			    ypclnterr2string(err) );
			exit(1);
		}
	}

	while (TRUE) {

		if (dumpkeys) {

			for (scan = outkey; *scan != NULL; scan++) {

				if (*scan == '\n') {
					*scan = ' ';  
				}
			}

			printf(outkey);
		}

		printf(val);
		free(val);
		key = outkey;
		keylen = outkeylen;
		
		if (err = getnext(domain, map, key, keylen, &outkey, &outkeylen,
		    &val, &vallen) ) {

			if (err == YPERR_NOMORE) {
				break;
			} else {
				fprintf(stderr, err_next_failed,
				    ypclnterr2string(err) );
				exit(1);
			}
		}

		free(key);
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

	argv++;
	
	while (--argc > 1) {

		if ( (*argv)[0] == '-') {

			switch ((*argv)[1]) {

			case 'a': {
				getfirst = ypclnt_first;
				getnext = ypclnt_next;
				argv++;
				break;
			}
				

			case 't': {
				translate = FALSE;
				argv++;
				break;
			}
				

			case 'k': {
				dumpkeys = TRUE;
				argv++;
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
			fprintf(stderr, err_usage);
			exit(1);
		}
	}

	if (argc == 1) {
		
		if ( (*argv)[0] == '-') {
			fprintf(stderr, err_usage);
			exit(1);
		}
		
		map = *argv;

		if (strlen(map) > YPMAXMAP) {
			fprintf(stderr, err_bad_args,
			    err_bad_mapname);
			exit(1);
		}

	} else {
		fprintf(stderr, err_usage);
		exit(1);
	}
}
/*
 * This maps a ypclnt error code into a printable string for inclusion in
 * a logging error message.
 */
static char *
ypclnterr2string(err)
	unsigned int err;
	
{
	char *p;

	switch (err) {

	case YPERR_BADARGS:  {
		p = "args to function are bad";
		break;
	}

	case YPERR_RPC:  {
		p = "RPC failure";
		break;
	}

	case YPERR_DOMAIN:  {
		p = "can't bind to a server which serves this domain";
		break;
	}

	case YPERR_MAP:  {
		p = "no such map in server's domain";
		break;
	}

	case YPERR_KEY:  {
		p = "no such key in map";
		break;
	}

	case YPERR_YPERR:  {
		p = "internal yp server or client interface error";
		break;
	}

	case YPERR_RESRC:  {
		p = "local resource allocation failure";
		break;
	}

	case YPERR_NOMORE:  {
		p = "no more records in map database";
		break;
	}
	case YPERR_PMAP:  {
		p = "can't communicate with portmapper";
		break;
	}

	case YPERR_YPBIND:  {
		p = "can't communicate with ypbind";
		break;
	}

	case YPERR_YPSERV:  {
		p = "can't communicate with ypserv";
		break;
	}

	case YPERR_NODOM:  {
		p = "local domain name not set";
		break;
	}

	default: {
		p = "unknown yp client error code";
		break;
	}

	}

	return(p);
	
}
