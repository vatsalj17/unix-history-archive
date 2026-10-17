#ifndef lint
static	char sccsid[] = "@(#)ypserv_ancil.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * This contains ancillary functions used by the yellow pages server process.
 */

#include "ypsym.h"
struct peer_list_item *set_random_alternate();

static struct dom_binding ping_peer;
static struct peer_list_item *pingee = (struct peer_list_item *) NULL;
static struct timeval pingintertry = {	/* udp secs between tries on ping */
	YPINTERTRY_TIME,		/* Seconds */
	0				/* uSecs */
};
static struct timeval pingtimeout = {	/* udp total timeout for ping */
	YPPING_TIME,			/* Seconds */
	0				/* uSecs */
};
static unsigned short int xfr_number = 0;


/*
 * This constructs a file name from a passed domain name, a passed map name,
 * and a globally known yellow pages data base path prefix.
 *
 * Note:  No pathname separators are assumed in either the map name or
 * the domain name.
 */
void
ypmkfilename(pdomain, pmap, ppath)
	char *pdomain;
	char *pmap;
	char *ppath;
{

	if ( (strlen(pdomain) + strlen(pmap) + strlen(ypdbpath) + 3) 
	    > (MAXNAMLEN + 1) ) {
		fprintf(stderr, "ypserv:  Map name string too long.\n");
	}

	strcpy(ppath, ypdbpath);
	strcat(ppath, "/");
	strcat(ppath, pdomain);
	strcat(ppath, "/");
	strcat(ppath, pmap);
}

/*
 * This sets up an rpc channel to a named server.  The
 * underlying transport mechanism (tcp or udp) is specified as an input.
 *	char *transport;		* Either "tcp" or "udp" (a null-
 *					*   terminated string)
 *	struct dom_binding *pdomb;	* Ptr to a domain binding.  The internet
 *					*   address should be filled out.
 *	struct timeval *ptimeval;	* The time to be used as an intertry
 *					*   time on udp connections.
 * This assumes that both the internet address and the port
 * have been set up in the sockaddr_in which is in the dom_binding.  It
 * is OK for the port to be 0 (this will generate an extra transaction at
 * the rpc level, asking the port mapper on the remote node for the port
 * of the remote yp server), but a garbage port or address value will
 * not work.  Also, since the dom_binding is a passed parameter, there
 * is no socket or client management done at this level.
 * 
 * Returns TRUE if client channel could be set up and the server is alive,
 * else FALSE.  All resources allocated by the call will be freed in the
 * failure case.
 */
bool
ypbind_to_named_server(transport, pdomb, ptimeval)
	char *transport;
	struct dom_binding *pdomb;
	struct timeval *ptimeval;
{
	enum clnt_stat clnt_stat;

	if (!transport ||
	    (strcmp(transport, "tcp") && strcmp(transport, "udp") ) ||
	    !pdomb || !ptimeval) {
		return(FALSE);
	}

	pdomb->dom_server_addr.sin_family = AF_INET;
	pdomb->dom_socket = RPC_ANYSOCK;

	if (!strcmp (transport, "tcp")) {

		if ((pdomb->dom_client =
		    clnttcp_create (&(pdomb->dom_server_addr), YPPROG, YPVERS,
	    	    &(pdomb->dom_socket), 1024, 1024)) == NULL) {
			return(FALSE);
		}
		
	} else {

		if ((pdomb->dom_client =
		    clntudp_create (&(pdomb->dom_server_addr), YPPROG, YPVERS,
		    *ptimeval, &(pdomb->dom_socket))) == NULL) {
			return(FALSE);
		}
	}

	pdomb->dom_server_port = pdomb->dom_server_addr.sin_port;

	if ( (clnt_stat =
	    (enum clnt_stat) clnt_call(pdomb->dom_client, YPPROC_NULL, xdr_void,
	    0, xdr_void, 0, pingtimeout) ) != RPC_SUCCESS) {
		clnt_destroy(pdomb->dom_client);
		close(pdomb->dom_socket);
		return (FALSE);
	}
	
	return (TRUE);			/* This is the go path */
}

/*
 * This returns for a name the integer index of that name within an array of
 * names.  If the passed name is not in the array, the function will return -1.
 * This is used both for checking to see if a name is on the list, and to use the
 * returned value as an index into an array parallel to the name array.  Returns
 * an index if match found, or -1 otherwise.
 * 
 * Note:  The list is assumed to be NULL-terminated.
 */
int
ypspecial_casep(name, list)
	char *name;
	char *list[];
{
	int index = 0;

	if (!name || (strlen(name) == 0) || !list) {
		return(-1);
	}
	
	while (*list) {
		
		if (!strcmp(name, *list)) {
			break;
		}

		index++;
		list++;
	}
		
	if (*list) {
		return(index);
	} else {
		return(-1);
	}
}

/*
 * This sends an "Are you alive" message to the yp server at some node, setting
 * the peer_port field at the same time.  It filters peers, and won't send the
 * message to itself, but will rather mark the "local peer" as reachable.  The
 * peer list item fields peer_port, peer_reachable, and peer_last_polled may be
 * set.
 */
void
ypping_peer(ppeer)
	struct peer_list_item *ppeer;
{
	enum clnt_stat clnt_stat;
	
	if (!ppeer) {
		return;
	}
		
	/*
	 * We are going to say that we are polling now, success or failure.
	 * We are going to assume failure.
	 */

	ppeer->peer_last_polled = tick_counter;
	ppeer->peer_reachable = FALSE;

	/*
	 * If the peer in question is me, mark me as reachable and return.
	 */

	if (!strcmp(ppeer->peer_pname, myhostname) ) {
		ppeer->peer_reachable = TRUE;
		return;
	}
	 
	/*
	 * If we are already bound to this peer, we'll try to use the
	 * binding.  If we use the binding, and are successful, we will
	 * mark the peer as reachable and return.  If the ping fails, we will
	 * clear the peer's port, in case he or his machine crashed, and
	 * his port has changed.
	 */
	 
	if (ppeer == pingee) {

		if ( (clnt_stat = (enum clnt_stat) clnt_call(
		    ping_peer.dom_client, YPPROC_NULL, xdr_void, 0, xdr_void,
		    0, pingtimeout) ) == RPC_SUCCESS) {
			ppeer->peer_reachable = TRUE;
			return;
		} else {
			ppeer->peer_port = htons( (unsigned short) YPNOPORT);
		}
	}

	/*
	 * Either we need to ping a server who was not the previous pingee, or
	 * our use of the old binding failed.  If we have a binding active
	 * we need to pull it down and free the associated resources.  
	 */

	if (pingee) {
		clnt_destroy(ping_peer.dom_client);
		close(ping_peer.dom_socket);
		pingee = (struct peer_list_item *) NULL;
	}

	/*
	 * Try to make a binding to the named peer.  (It's not me.)  If the
	 * binding succeeds, the server has already been successfully pinged
	 * by ypbind_to_named_server.  We will suck up the server's port, (in 
	 * case it was in its initial state) and remember the current binding.
	 */

	ping_peer.dom_server_addr.sin_addr.s_addr = ppeer->peer_addr.s_addr;
	ping_peer.dom_server_addr.sin_port = ppeer->peer_port;

	if (ypbind_to_named_server(peer_transport, &ping_peer,
	    &pingintertry) ) {
		pingee = ppeer;
		ppeer->peer_port = ping_peer.dom_server_port;
		ppeer->peer_reachable = TRUE;
	} else {
		ppeer->peer_port = htons( (unsigned short) YPNOPORT);
	}
}

/*
 * Runs though all known peers in all supported domains and pings them.  This
 * is called only at initialization time.  The peer_reachable and the
 * peer_last_polled fields on the peers may change state.  All work is done at
 * lower levels.
 */
void
ypping_all_peers()
{
	struct domain_list_item *pdom;
	struct peer_list_item *ppeer;

	for (pdom = yppoint_at_first_domain(TRUE);
	    pdom != (struct domain_list_item *) NULL;
	    pdom = yppoint_at_next_domain(pdom, TRUE) ) {

		for (ppeer = yppoint_at_peerlist(pdom);
		    ppeer != (struct peer_list_item *) NULL;
		    ppeer = ypnext_peer(ppeer) ) {
			ypping_peer(ppeer);
		}
	}
}


/*
 * This is a boolean function which attempts to find a "suitable" alternate
 * peer to serve a passed map.  It searches through the list of peers in the
 * domain and tries to return a peer which is:
 * 	1.  Not the map master
 * 	2.  Not myself
 * 	3.  Reachable
 *
 * The field pmap->map_alternate will be set to the alternate
 * peer if the functional value is TRUE, otherwise the field will be set to
 * NULL.  Field peer_reachable for the alternate will be set true by lower
 * levels.
 */
bool
ypfind_alternate(pmap)
	struct map_list_item *pmap;
{
	struct peer_list_item *ppeer;
	struct peer_list_item *peerlist;
	int peercount = 0;

	if (!pmap) {
		return(FALSE);
	}

	pmap->map_alternate = (struct peer_list_item *) NULL;
	
	/*
	 * Traverse the list of peers, counting and  marking as reachable
	 * all except myself and the map's master.
	 */

	for (peerlist = ppeer = yppoint_at_peerlist(pmap->map_domain);
	    ppeer != (struct peer_list_item *) NULL;
	    ppeer = ypnext_peer(ppeer) ) {

		if (ppeer == pmap->map_master) {
			ppeer->peer_reachable = FALSE;
		} else if (!strcmp(ppeer->peer_pname, myhostname) ) {
			ppeer->peer_reachable = FALSE;
		} else {
			peercount++;
			ppeer->peer_reachable = TRUE;
		}
	}

	if (peercount == 0) {
		return(FALSE);
	}
	
	ppeer = (struct peer_list_item *) NULL;

	/*
	 * Choose an assumed reachable peer "at random", and test to see if
	 * he really is reachable. If he is, use him as the alternate; if he's
	 * not, knock him out of the list and try till we've got a reachable
	 * peer, or we've tried them all.
	 */

	while ( (ppeer == (struct peer_list_item *) NULL) &&
	    (peercount > 0) ) {
		ppeer = set_random_alternate(peerlist, peercount);

		if (ppeer) {
			ypping_peer(ppeer);

			if (ppeer->peer_reachable) {
				pmap->map_alternate = ppeer;
			} else {
				peercount--;
				ppeer = (struct peer_list_item *) NULL;
			}
			
		} else {
			break;
		}
	}

	return(ppeer ? TRUE: FALSE);
}

/*
 * This chooses one of the peers which is marked as reachable from the passed
 * peerlist.  The peer chosen is "at random".
 */
 
static struct peer_list_item *
set_random_alternate(peerlist, peercount)
	struct peer_list_item *peerlist;
	int peercount;
	
{
	struct timeval tod;
	int index;
	int match = 0;
	struct peer_list_item *ppeer;

	if (!peerlist || (peercount == 0) ) {
		return( (struct peer_list_item *) NULL);
	}
	 
	/*
	 * Generate a quick pseudorandom number between 0 and peercount - 1, and
	 * use it as an index into the list of (assumed) reachable peers.
	 */

	gettimeofday(&tod, NULL);
	index = tod.tv_usec % peercount;

	for (ppeer = peerlist;
	    ppeer != (struct peer_list_item *) NULL;
	    ppeer = ypnext_peer(ppeer)) {

		if (ppeer->peer_reachable) {

			if (match == index) {
				return(ppeer);
			} else {
				match++;
			}
		}
	}

	return( (struct peer_list_item *) NULL);
}

/*
 * This returns a pointer to a temporary name for a map transfer.
 */
void
ypmk_tmpname(xfr_name)
	char *xfr_name;
{
	int len;
	char xfr_anumber[10];

	if (!xfr_name) {
		return;
	}
	
	strcpy(xfr_name, YPTEMPNAME_PREFIX);
	sprintf(xfr_anumber, "%d", xfr_number++);
	strcat(xfr_name, xfr_anumber);
}

/*
 * This checks to see if the source map files exist, then renames them to the
 * target names.  This is a boolean function.  The file names from.pag and
 * from.dir will be changed to to.pag and to.dir in the success case.
 *
 * Note:  If the second of the two renames fails, yprename_map will try to
 * un-rename the first pair, and leave the world in the state it was on entry.
 * This might fail, too, though...
 */
bool
yprename_map(from, to)
	char *from;
	char *to;
{
	char fromfile[MAXNAMLEN + 1];
	char tofile[MAXNAMLEN + 1];

	if (!from || !to) {
		return(FALSE);
	}
	
	if (!ypcheck_map_existence(from) ) {
		return(FALSE);
	}

	strcpy(fromfile, from);
	strcat(fromfile, ".pag");
	strcpy(tofile, to);
	strcat(tofile, ".pag");

	if (rename(fromfile, tofile) ) {
		fprintf(stderr, "ypserv:  yprename_map: can't mv %s to %s.\n",
		    fromfile, tofile);
		return(FALSE);
	}
	
	strcpy(fromfile, from);
	strcat(fromfile, ".dir");
	strcpy(tofile, to);
	strcat(tofile, ".dir");

	if (rename(fromfile, tofile) ) {
		fprintf(stderr, "ypserv:  yprename_map: can't mv %s to %s.\n",
		    fromfile, tofile);

		strcpy(fromfile, from);
		strcat(fromfile, ".pag");
		strcpy(tofile, to);
		strcat(tofile, ".pag");

		if (rename(tofile, fromfile) ) {
			fprintf(stderr,
			    "ypserv:  yprename_map: can't recover.\n");
			return(FALSE);
		}
	
		
		return(FALSE);
	}

	return(TRUE);
	
}

/*
 * This deletes the .pag and .dir files which implement a map.
 *
 * Note:  No error checking is done here for a garbage input file name or for
 * failed unlink operations.
 */
void
ypdel_mapfiles(basename)
	char *basename;
{
	char dbfilename[MAXNAMLEN + 1];

	if (!basename) {
		return;
	}
	
	strcpy(dbfilename, basename);
	strcat(dbfilename, ".pag");
	unlink(dbfilename);
	strcpy(dbfilename, basename);
	strcat(dbfilename, ".dir");
	unlink(dbfilename);
}

/*
 * This provides a layer above ypbind_to_named_peer which knows about the
 * dom_binding at xfr_binding and the associated peer pointer xfr_peer.  It
 * does the socket and client management required when changing from one peer
 * to another.  The dom_binding at xfr_binding will be set up to reflect
 * the rpc path to the peer, and xfr_peer will be set to point to the peer.
 * In the failure case, xfr_peer will be set to NULL, and no rpc path will
 * be valid.
 *
 * Note:  If the transfer peer binding is already set up to point to the input
 * peer, the function will return TRUE without testing the binding.  If the
 * binding is tested, however, an entire "ping" will be done, including
 * resetting the peer_last_polled, peer_reachable, and peer_port fields to
 * current values.
 */
bool
ypset_xfr_peer(ppeer)
	struct peer_list_item *ppeer;
{
	if (!ppeer) {
		return(FALSE);
	}
	
	if (xfr_peer == ppeer) {
		return(TRUE);
	}

	ypclr_xfr_peer();
	xfr_binding.dom_server_addr.sin_addr.s_addr = ppeer->peer_addr.s_addr;
	xfr_binding.dom_server_addr.sin_port = ppeer->peer_port;
	ppeer->peer_last_polled = tick_counter;
	ppeer->peer_reachable = FALSE;
		
	if (ypbind_to_named_server(peer_transport, &xfr_binding,
	    &ypintertry) ) {
		xfr_peer = ppeer;
		ppeer->peer_port = ping_peer.dom_server_port;
		ppeer->peer_reachable = TRUE;
		return(TRUE);
	} else {
		ppeer->peer_port = htons( (unsigned short) YPNOPORT);
		return(FALSE);
	}
}

/*
 * This closes the socket associated with the transfer peer data structures,
 * and tears down the rcp path, releasing the associated system port.
 */
void
ypclr_xfr_peer()
{
	if (xfr_peer) {
		clnt_destroy(xfr_binding.dom_client);
		close(xfr_binding.dom_socket);
		xfr_peer = (struct peer_list_item *) NULL;
	} else {
		return;
	}
}

/*
 * This asks the current transfer peer for the order number associated with a
 * passed map.  If it gets the order number value back, it converts the value
 * to an int and passes that back as an output value.
 *
 * Notes:   If this function is called when xfr_peer is set to NULL, it is an
 * error condition.  In that case, a functional value of YPERR_YPERR (internal
 * error) will be returned.
 */
int
yppoll_for_order_number(pmap, order_number)
	struct map_list_item *pmap;
	int *order_number;
{
	struct yprequest req;
	struct ypresponse resp;
	unsigned int retval = 0;
	bool go_on = TRUE;
	enum clnt_stat clnt_stat;

	if (!pmap) {
		return(YPERR_YPERR);
	}
	
	pmap->map_last_polled = tick_counter;
	
	if (!order_number || !xfr_peer) {
		return(YPERR_YPERR);
	}

	req.yppoll_req_domain = pmap->map_domain->dom_name;
	req.yppoll_req_map = pmap->map_name;
	req.yp_reqtype = YPPOLL_REQTYPE;
	
	resp.yppoll_resp_domain = NULL;
	resp.yppoll_resp_map = NULL;
	resp.yppoll_resp_owner = NULL;
	resp.yppoll_resp_ordernum = 0;

	
	if( (clnt_stat = (enum clnt_stat) clnt_call(xfr_binding.dom_client,
	    YPPROC_POLL, xdr_yprequest, &req, xdr_ypresponse, &resp,
	    yptimeout) ) != RPC_SUCCESS) {
		return(YPERR_RPC);
	}

	if (resp.yp_resptype == YPPOLL_RESPTYPE) {

		if (strlen(resp.yppoll_resp_domain) == 0) {
			retval = YPERR_DOMAIN;
			go_on = FALSE;
		}
		
		if (go_on && strlen(resp.yppoll_resp_map) == 0) {
			retval = YPERR_MAP;
			go_on = FALSE;
		}

		if (go_on && resp.yppoll_resp_ordernum == 0) {
			retval = YPERR_KEY;
			go_on = FALSE;
		}

		if (go_on) {
			*order_number = resp.yppoll_resp_ordernum;
		}
		
	} else {
		retval = YPERR_YPERR;
	}

	CLNT_FREERES(xfr_binding.dom_client, xdr_ypresponse, &resp);
	return(retval);
}

/*
 * This tries to find out whether a map should be queued to the map transfer
 * list.  It should be if it doesn't exist, or if it does exist, but is not
 * supported, or if its master is not known.  Maps of which we are the master
 * will be checked locally to catch points at which the humans running around
 * the system have changed the world beneath us.  No RPC-based stuff is being
 * done here.
 */
void
ypping_map(pmap)
	struct map_list_item *pmap;
{
	struct peer_list_item *ppeer = (struct peer_list_item *) NULL;
	unsigned long order_at_peer;
	char map[MAXNAMLEN + 1];
	bool was_supported;
	unsigned long old_order;
	int case_index;
	int pid;

	if (!pmap) {
		return;
	}
	
	pmap->map_last_polled = tick_counter;
	ppeer = pmap->map_master;

	if (ppeer && (strcmp(ppeer->peer_pname, myhostname) == 0) ) {
		was_supported = pmap->map_supported;
		old_order = pmap->map_order;
		ypmkfilename(pmap->map_domain->dom_name, pmap->map_name, map);

		if (ypcheck_map_existence(map) ) {
			pmap->map_exists = TRUE;

			if(ypget_map_order(pmap) ) {
				pmap->map_supported = TRUE;
			} else {
				pmap->map_supported = FALSE;
			}
				
		} else {
			pmap->map_exists = FALSE;
			pmap->map_supported = FALSE;
		}

		if (pmap->map_supported && ( (!was_supported) ||
		    (was_supported && (old_order != pmap->map_order) ) ) ) {
			    
			/*
		 	 * If the map is in the list of special cases,
			 * call the special case handler.
		 	 */
		 
			if ( (case_index = ypspecial_casep(
			    pmap->map_name, map_special_cases) ) >= 0) {
				special_map_handlers[case_index](pmap);
			}

			pid = fork();

			if (pid == 0) {
				ypclr_xfr_peer();

				for (ppeer =
				    yppoint_at_peerlist(pmap->map_domain);
			    	    ppeer != (struct peer_list_item *) NULL;
			    	    ppeer = ypnext_peer(ppeer)) {

					if (strcmp(ppeer->peer_pname,
					    myhostname) == 0) {
						continue;
				 	}
						 
					ypsend_getreq(ppeer, pmap);
				}

				exit(0);
			}
		}

	} else {			/* Master unknown, or not me */
		(void) ypadd_xfr(pmap);
	}
}
