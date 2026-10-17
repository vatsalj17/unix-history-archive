#ifndef lint
static	char sccsid[] = "@(#)ypserv_proc.c 1.1 85/05/30 Copyr 1985 Sun Micro";
#endif

/*
 * This contains yellow pages server code which supplies
 * the set of functions which are requested using the rpc interface defined
 * in yp_prot.h   The top level functions in this module are those which
 * have symbols of the form YPPROC_xxxx in yp_prot.h.  This module also
 * contains functions which are used by (and only by) the top-level
 * functions here.
 */

#include "ypsym.h"
bool check_map_to_push();

/*
 * This implements the yp "match" function.
 */
void
ypmatch(rqstp, transp) 
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	char mapname[YPDBPATH_LENGTH + YPMAXDOMAIN + YPMAXMAP + 3];
	bool dbmop_ok = TRUE;
	struct yprequest req;
	struct ypresponse resp;
	
	req.ypmatch_req_domain = req.ypmatch_req_map = NULL;
	req.ypmatch_req_keyptr = NULL;
	resp.ypmatch_resp_valptr = NULL;
	resp.ypmatch_resp_valsize = 0;

	if (!svc_getargs(transp, xdr_yprequest, &req) ) {
		svcerr_decode(transp);
		return;
	}

	if (req.yp_reqtype != YPMATCH_REQTYPE) {
		resp.ypmatch_resp_status = YP_BADARGS;
		dbmop_ok = FALSE;
	};

	if (dbmop_ok && ypset_current_map(req.ypmatch_req_map,
	    req.ypmatch_req_domain, &resp.ypmatch_resp_status) ) {
		    
		resp.ypmatch_resp_valdat = fetch(req.ypmatch_req_keydat);

		if (resp.ypmatch_resp_valptr != NULL) {
			resp.ypmatch_resp_status = YP_TRUE;
		} else {
			resp.ypmatch_resp_status = YP_NOKEY;
		}

	}

	resp.yp_resptype = YPMATCH_RESPTYPE;

	if (!svc_sendreply(transp, xdr_ypresponse, &resp) ) {
		fprintf(stderr, "ypserv:  Can't respond to rpc request.\n");
	}

	if (!svc_freeargs(transp, xdr_yprequest, &req) ) {
		fprintf(stderr, "ypserv:  ypmatch can't free args.\n");
	}

}

/*
 * This implements the yp "get first" function.
 */
void
ypfirst(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	char mapname[YPDBPATH_LENGTH + YPMAXDOMAIN + YPMAXMAP + 3];
	bool dbmop_ok = TRUE;
	struct yprequest req;
	struct ypresponse resp;
	
	req.ypfirst_req_domain = req.ypfirst_req_map = NULL;
	resp.ypfirst_resp_keyptr = resp.ypfirst_resp_valptr = NULL;
	resp.ypfirst_resp_keysize = resp.ypfirst_resp_valsize = 0;

	if (!svc_getargs(transp, xdr_yprequest, &req) ) {
		svcerr_decode(transp);
		return;
	}

	if (req.yp_reqtype != YPFIRST_REQTYPE) {
		resp.ypfirst_resp_status = YP_BADARGS;
		dbmop_ok = FALSE;
	};

	if (dbmop_ok && ypset_current_map(req.ypfirst_req_map,
	    req.ypfirst_req_domain, &resp.ypfirst_resp_status) ) {

		resp.ypfirst_resp_keydat = firstkey();

		if (resp.ypfirst_resp_keyptr != NULL) {
			resp.ypfirst_resp_valdat =
			    fetch(resp.ypfirst_resp_keydat);

			if (resp.ypfirst_resp_valptr != NULL) {
				resp.ypfirst_resp_status = YP_TRUE;
			} else {
				resp.ypfirst_resp_status = YP_BADDB;
			}

		} else {
			resp.ypfirst_resp_status = YP_NOKEY;
		}
	}

	resp.yp_resptype = YPFIRST_RESPTYPE;

	if (!svc_sendreply(transp, xdr_ypresponse, &resp) ) {
		fprintf(stderr, "ypserv:  Can't respond to rpc request.\n");
	}

	if (!svc_freeargs(transp, xdr_yprequest, &req) ) {
		fprintf(stderr, "ypserv:  ypfirst can't free args.\n");
	}
}

/*
 * This implements the yp "get next" function.
 */
void
ypnext(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	char mapname[YPDBPATH_LENGTH + YPMAXDOMAIN + YPMAXMAP + 3];
	bool dbmop_ok = TRUE;
	struct yprequest req;
	struct ypresponse resp;
	
	req.ypnext_req_domain = req.ypnext_req_map = NULL;
	req.ypnext_req_keyptr = NULL;
	resp.ypnext_resp_keyptr = resp.ypnext_resp_valptr = NULL;
	resp.ypnext_resp_keysize = resp.ypnext_resp_valsize = 0;

	if (!svc_getargs(transp, xdr_yprequest, &req) ) {
		svcerr_decode(transp);
		return;
	}

	if (req.yp_reqtype != YPNEXT_REQTYPE) {
		resp.ypnext_resp_status = YP_BADARGS;
		dbmop_ok = FALSE;
	};

	if (dbmop_ok && ypset_current_map(req.ypnext_req_map,
	    req.ypnext_req_domain, &resp.ypnext_resp_status) ) {
		resp.ypnext_resp_keydat = nextkey(req.ypnext_req_keydat);

		if (resp.ypnext_resp_keyptr != NULL) {
			resp.ypnext_resp_valdat =
			    fetch(resp.ypnext_resp_keydat);

			if (resp.ypnext_resp_valptr != NULL) {
				resp.ypnext_resp_status = YP_TRUE;
			} else {
				resp.ypnext_resp_status = YP_BADDB;
			}

		} else {
			resp.ypnext_resp_status = YP_NOMORE;
		}

	}
	
	resp.yp_resptype = YPNEXT_RESPTYPE;

	if (!svc_sendreply(transp, xdr_ypresponse, &resp) ) {
		fprintf(stderr, "ypserv:  Can't respond to rpc request.\n");
	}

	if (!svc_freeargs(transp, xdr_yprequest, &req) ) {
		fprintf(stderr, "ypserv:  ypnext can't free args.\n");
	}

}

/*
 * This retrieves the order number and master peer name from the internal data
 * base.  The conditions for the various message fields are:
 * 	domain is filled in iff the domain is supported.
 *	map is filled in iff the map exists.
 * 	order number is filled in iff the map is supported.
 * 	owner is filled in iff the master peer is known.
 */
void
yppoll(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	struct yprequest req;
	struct ypresponse resp;
	char *map = "";
	char *domain = "";
	char *owner = "";
	struct domain_list_item *pdom;
	struct map_list_item *pmap;

	req.yppoll_req_domain = req.yppoll_req_map = NULL;
	
	resp.yp_resptype = YPPOLL_RESPTYPE;
	resp.yppoll_resp_domain = domain;
	resp.yppoll_resp_map = map;
	resp.yppoll_resp_owner = owner;
	resp.yppoll_resp_ordernum = 0;

	if (!svc_getargs(transp, xdr_yprequest, &req) ) {
		svcerr_decode(transp);
		return;
	}

	if (req.yp_reqtype == YPPOLL_REQTYPE) {

		if (pdom = ypcheck_domain(req.yppoll_req_domain) ) {
			domain = pdom->dom_name;

			if (pmap = yppoint_at_map(req.yppoll_req_map, pdom) ) {
				map = pmap->map_name;

				if (pmap->map_supported) {
					resp.yppoll_resp_ordernum =
					    pmap->map_order;

					if (pmap->map_master) {
						owner =
						    pmap->map_master->peer_pname;
					}
				}
			}
		}
	}
	
	resp.yppoll_resp_domain = domain;
	resp.yppoll_resp_map = map;
	resp.yppoll_resp_owner = owner;

	if (!svc_sendreply(transp, xdr_ypresponse, &resp) ) {
		fprintf(stderr,
		    "ypserv:  yppoll can't respond to rpc request.\n");
	}

	if (!svc_freeargs(transp, xdr_yprequest, &req) ) {
		fprintf(stderr, "ypserv:  yppoll can't free args.\n");
	}
}

/*
 * This checks to see if the pull request refers to an existing map, and, if it
 * does, adds that map to the map transfer list.
 */
void
yppull(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	struct yprequest req;
	struct domain_list_item *pdom;
	struct map_list_item *pmap;

	req.yppull_req_domain = req.yppull_req_map = NULL;

	if (!svc_getargs(transp, xdr_yprequest, &req) ) {
		svcerr_decode(transp);
		return;
	}

	if (!svc_sendreply(transp, xdr_void, 0) ) {
		fprintf(stderr,
		    "ypserv:  yppull can't respond to rpc request.\n");
	}

	if (req.yp_reqtype == YPPULL_REQTYPE) {

		if (pdom = ypcheck_domain(req.yppull_req_domain) ) {

			if (pmap = yppoint_at_map(req.yppull_req_map, pdom) ) {
				pmap->map_last_polled = 0;
				(void) ypadd_xfr(pmap);
			}
		}
	}

	if (!svc_freeargs(transp, xdr_yprequest, &req) ) {
		fprintf(stderr, "ypserv:  yppull can't free args.\n");
	}
}

/*
 * This checks to see if the named map is one which we know anything about, is
 * one which exists, and is supported, and is one for which we are the master.
 * If all conditions hold, we attempt to contact each peer in the domain,
 * sending each a "get" request, listing ourselves as the map master.
 */
void
yppush(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	struct yprequest req;
	struct domain_list_item *pdom;
	struct map_list_item *pmap;
	struct peer_list_item *ppeer;
	int pid;

	req.yppush_req_domain = req.yppush_req_map = NULL;

	if (!svc_getargs(transp, xdr_yprequest, &req) ) {
		svcerr_decode(transp);
		return;
	}

	if (!svc_sendreply(transp, xdr_void, 0) ) {
		fprintf(stderr,
		    "ypserv:  yppush can't respond to rpc request.\n");
	}

	if (req.yp_reqtype == YPPUSH_REQTYPE) {

		if (pdom = ypcheck_domain(req.yppush_req_domain) ) {

			if (pmap = yppoint_at_map(req.yppush_req_map, pdom) ) {
				
				if (check_map_to_push(pmap) ) {
					pid = fork();

					if (pid == 0) { 

					ypclr_xfr_peer();

					for (ppeer =  yppoint_at_peerlist(pdom);
					    ppeer != (struct peer_list_item *)
					    NULL;
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
			}   
		}
	}

	if (!svc_freeargs(transp, xdr_yprequest, &req) ) {
		fprintf(stderr, "ypserv:  yppush can't free args.\n");
	}
}

/*
 * This checks to see if the map which we are being asked to push exists and is
 * supportable, and keeps track of any changes in the map parameters which show
 * up while doing the checking.  If the world has changed, it checks to see if
 * the map is one requiring special processing, and if so, calls the appropriate
 * special case map handler.
 */
static bool
check_map_to_push(pmap)
	struct map_list_item *pmap;
{
	struct peer_list_item *pmaster;
	char map[MAXNAMLEN + 1];
	bool was_supported;
	unsigned long old_order;
	int case_index;
	
	pmaster = pmap->map_master;

	if (pmaster && (strcmp(pmaster->peer_pname, myhostname) == 0) ) {
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

			if (pmap->map_supported && ( (!was_supported) ||
			    (was_supported && (old_order != pmap->map_order)))) {
			    
				/*
		 	 	* If the map is in the list of special cases,
				* call the special case handler.
		 	 	*/
		 
				if ( (case_index =
				    ypspecial_casep(pmap->map_name,
			    	    map_special_cases)) >= 0) {
					special_map_handlers[case_index](pmap);
				}
			}

			if (pmap->map_supported) {
				return(TRUE);
			} else {
				return(FALSE);
			}
		}
		
	} else {
		return(FALSE);
	}
}

/*
 * Does the almost the same thing as yppull, but replaces the existing master
 * peer name with that of the request.  The exception is the case in which we
 * have no knowledge at all of a peer; we can't make him the master if we don't
 * know his internet address.
 *
 * Note:  The master name field is filled in on a sort of provisional basis:
 * the next time map ypmaps comes in, the master name field will be set to the
 * gospel as represented by that map.
 */
void
ypget(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	struct yprequest req;
	struct domain_list_item *pdom;
	struct map_list_item *pmap;
	struct peer_list_item *ppeer;

	req.ypget_req_domain = req.ypget_req_map = req.ypget_req_owner = NULL;
	req.ypget_req_ordernum = 0;
	
	if (!svc_getargs(transp, xdr_yprequest, &req) ) {
		svcerr_decode(transp);
		return;
	}

	if (!svc_sendreply(transp, xdr_void, 0) ) {
		fprintf(stderr,
		    "ypserv:  ypget can't respond to rpc request.\n");
	}

	if (req.yp_reqtype == YPGET_REQTYPE) {

		if (pdom = ypcheck_domain(req.ypget_req_domain) ) {

			if (pmap = yppoint_at_map(req.ypget_req_map, pdom) ) {

				if (ppeer =
				    yppoint_at_peer(req.ypget_req_owner, pdom)) {
					pmap->map_master = ppeer;
				}
				
				pmap->map_last_polled = 0;
				(void) ypadd_xfr(pmap);
			}
		}
	}

	if (!svc_freeargs(transp, xdr_yprequest, &req) ) {
		fprintf(stderr, "ypserv:  ypget can't free args.\n");
	}
}

/*
 * This determines whether or not a passed domain is served by this server,
 * and returns a boolean.
 */
void
ypdomain(rqstp, transp, always_respond)
	struct svc_req *rqstp;
	SVCXPRT *transp;
	bool always_respond;
{
	char domain_name[YPMAXDOMAIN + 1];
	char *pdomain_name = domain_name;
	bool isserved;

	if (!svc_getargs(transp, xdr_ypdomain_wrap_string, &pdomain_name) ) {
		svcerr_decode(transp);
		return;
	}

	isserved = (bool) ypcheck_domain(domain_name);

	if (isserved || always_respond) {
		
		if (!svc_sendreply(transp, xdr_bool, &isserved) ) {
			fprintf(stderr,
			    "ypserv:  Can't respond to rpc request.\n");
		}

	} else {
		/*
		 * This case is the one in which the domain is not
		 * supported, and in which we are not to respond in the
		 * unsupported case.  We are going to make an error happen
		 * to allow the portmapper to end his wait without the
		 * normal udp timeout period.  The assumption here is that
		 * the only process in the world which is using the function
		 * in its no-answer-if-nack form is the portmapper, which is
		 * doing the krock for pseudo-broadcast.  If some poor fool
		 * calls this function as a single-cast message, the nack
		 * case will look like an incomprehensible error.  Sigh...
		 * (The traditional Unix disclaimer)
		 */
	
		svcerr_decode(transp);
	}

	if (!svc_freeargs(transp, xdr_ypdomain_wrap_string, &pdomain_name) ) {
		fprintf(stderr, "ypserv:  ypdomain can't free args.\n");
	}

}
