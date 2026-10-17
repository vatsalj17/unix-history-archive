#ifndef lint
static	char sccsid[] = "@(#)ypserv.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * ypserv.c
 * This contains the mainline code for the yellowpages server; functions which
 * are called by name only from the mainline; and functions which are called
 * as software signal handlers.  Data structures which are truly process-global
 * are also in this module.
 */

#include "ypsym.h"
#include <sys/ioctl.h>

bool ypdb_xfer_done = FALSE;		/* Set TRUE by ypxfr_proc_exit, 
					 *   FALSE by ypmap_xfer_done */
struct dom_binding xfr_binding;		/* Binding to transfering peer */
struct peer_list_item *xfr_peer = (struct peer_list_item *) NULL;
struct map_xfr_entry *map_xfr_list = (struct map_xfr_entry *) NULL;
struct map_xfr_entry *current_transfer;
bool ypinitialization_done = FALSE;
static int yptime_quantum  = YPTIME_QUANTUM; /* Time between alarm clock
					*   interrupts in seconds. */
unsigned long tick_counter = 0;		/* Ticktocktracker. */
struct timer_action alarm_actions[] = {
	{
		ypping_eligible_map,
		0,
		1
	},
	{				/* Last entry on the list always */
		(PFV) NULL,
		0,
		0
	}
	
};
static char create_failed[] = "ypserv:  Unable to create server for ";
static char register_failed[] = "ypserv:  Unable to register service for ";
char ypdbpath[] = __YP_PATH_PREFIX;
char peer_transport[] = "udp";
char order_key[] = ORDER_KEY;
char ypprivate_domain_name[] = YPPRIVATE_DOMAIN_NAME;
char ypdomains[] = YPDOMAINS;
struct timeval ypintertry = {		/* udp secs between tries in peer comm */
	YPINTERTRY_TIME,		/* Seconds */
	0				/* uSecs */
};
struct timeval yptimeout = {		/* udp total timeout for peer comm */
	YPTOTAL_TIME,			/* Seconds */
	0				/* uSecs */
};
char *domain_special_cases[] = {
	YPPRIVATE_DOMAIN_NAME,
	(char *) NULL			/* Last entry on the list always */
};

/* Tables map_special_cases and special_map_handlers must be parallel */

char *map_special_cases[] = {
	YPDOMAINS,
	YPSERVERS,
	YPMAPS,
	YPHOSTS_BYNAME,
	(char *) NULL			/* Last entry on the list always */
};
PFV special_map_handlers[] = {
	ypnew_ypdomains,
	ypnew_ypservers,
	ypnew_ypmaps,
	ypnew_hosts,
	(PFV) NULL
};
char myhostname[256];
SVCXPRT *udphandle;
SVCXPRT *tcphandle;
struct domain_list_item *pingpeer_curr_domain = NULL;
struct domain_list_item *pingmap_curr_domain = NULL;
	
#ifdef DEVELOPMENT
bool silent = FALSE;			/* Toggle to prevent process from forking
					 *   and turning off stderr messages */
#else
bool silent = TRUE;
#endif

bool log_transfers = FALSE;		/* Toggle to log the results of the
					 *   map transfers to a file in the
					 *   yp database directory.  This is
					 *   used by function ypdo_xfr in module
					 *   ypserv_xfr.c.  It is never set
					 *   TRUE by code; use adb to do it
					 *   before activating the process. */

void ypexit();

/*
 * This is the main line code for the yp server.  It registers the server with
 * rpc and the port mapper, and sets the first poll alarm.
 */
main()
{
	int readfds;
	struct timer_action *palarm_action;

 	ypinit(); 			/* Set up shop */

	/*
	 * This is the process' main loop.  On each iteration of the loop, the
	 * data base transfer-done flag is checked, and, if it is set,
	 * ypmap_xfer_done is called to relink the temp files to become the
	 * current map.  When that is done, the queue of maps to be updated
	 * is checked, and, if non-null, a map transfer process is kicked off
	 * to do the transfer.  Then the list of time - based functions is
	 * checked, and all which have reached their threshold value are
	 * called.  After all the houskeeping is done, the process does a
	 * blocked select on its rpc input queue.  When it wakes up as a result
	 * of a delivered message, it calls svc_retreq to get the rpc data to
	 * the yp dispatcher, which then calls the appropriate ypserv_proc
	 * function.
	 */

	for (;;) {

		if (ypdb_xfer_done) {
		    ypmap_xfer_done();
		}

		if ( (current_transfer == (struct map_xfr_entry *) NULL) &&
		   (map_xfr_list != (struct map_xfr_entry *) NULL) ) {
		    ypxfr_map();
		}

		for (palarm_action = alarm_actions;
			palarm_action->ta_action != NULL;
			palarm_action++) {

			if ((palarm_action->ta_ticks) ==
			    palarm_action->ta_threshold) {
				(palarm_action->ta_action) ();
				palarm_action->ta_ticks = 0;
		    	}
		}

		readfds = svc_fds;
		errno = 0;

		switch ( (int) select(32, &readfds, NULL, NULL, NULL) ) {

		case -1:  {
		
			if (errno != EINTR) {
			    fprintf (stderr,
			    "ypserv:  bad fds bits in main loop select mask.\n");
			}

			break;
		}

		case 0:  {
			fprintf (stderr,
			    "ypserv:  invalid timeout in main loop select.\n");
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
 * Does all startup processing for the yp server.  This includes setting up
 * shop as an rcp-based server, letting the portmapper know about us, setting
 * up the signal handlers, getting a list of all supported domains, all existing
 * domains, peer servers in each supported domain, maps in each domain, masters
 * for each map, and order numbers for each map.  A return from this function
 * implies that a usable level of functionality has been reached.  All error
 * reporting is done at this level or below.
 */
void
ypinit()
{
	struct timer_action *palarm_action;
	int pid;
	int t;

	if (silent) {
		
		pid = fork();
		
		if (pid == -1) {
			fprintf(stderr, "ypserv:  ypinit fork failure.\n");
			ypexit();
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

	gethostname(myhostname, 256);

	if ((udphandle = svcudp_create(RPC_ANYSOCK)) == (SVCXPRT *) NULL) {
		fprintf(stderr, "%s%s.\n", create_failed, "udp");
		ypexit();
	}

	if ((tcphandle = svctcp_create(RPC_ANYSOCK, 1024, 1024)) ==
	    (SVCXPRT *) NULL) {
		fprintf(stderr, "%s%s.\n", create_failed, "tcp");
		ypexit();
	}

	pmap_unset(YPPROG, YPVERS);

	if (!svc_register(udphandle, YPPROG, YPVERS, ypdispatch, IPPROTO_UDP) ) {
		fprintf(stderr, "%s%s.\n", register_failed, "udp");
		ypexit();
	}

	if (!svc_register(tcphandle, YPPROG, YPVERS, ypdispatch, IPPROTO_TCP) ) {
		fprintf(stderr, "%s%s.\n", register_failed, "tcp");
		ypexit();
	}

	if ((int) signal(SIGALRM, yptimer) == -1) {
		fprintf(stderr, "ypserv:  Can't catch alarm signal.\n");
		ypexit();
	}

	if ((int) signal(SIGCHLD, ypxfr_proc_exit) == -1) {
		fprintf(stderr,
		    "ypserv:  Can't catch xfr process exit signal.\n");
		ypexit();
	}

	if (!ypget_all_domains() ) {
		fprintf(stderr,
		    "ypserv:  ypinit can't build list of all domains.\n");
		ypexit();
	}
	
	ypget_supported_domains();
	ypbuild_peer_lists();
	ypget_all_maps();
	ypget_supported_maps();
/*	ypping_all_peers();	*/		/* Obsolete */
	ypinitialization_done = TRUE;
	alarm(yptime_quantum);			/* Start the motor running */
}

/*
 * This dispatches to server action routines based on the input procedure
 * number.  ypdispatch is called from the RPC function svc_getreq.
 */
void
ypdispatch(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{

	switch (rqstp->rq_proc) {

	case YPPROC_NULL:

		if (!svc_sendreply(transp, xdr_void, 0) ) {
			fprintf(stderr, "ypserv:  Can't reply to rpc call.\n");
		}

		break;

	case YPPROC_DOMAIN:
		ypdomain(rqstp, transp, TRUE);
		break;

	case YPPROC_DOMAIN_NONACK:
		ypdomain(rqstp, transp, FALSE);
		break;

	case YPPROC_MATCH:
		ypmatch(rqstp, transp);
		break;

	case YPPROC_FIRST:
		ypfirst(rqstp, transp);
		break;

	case YPPROC_NEXT:
		ypnext(rqstp, transp);
		break;

	case YPPROC_POLL:
		yppoll(rqstp, transp);
		break;

	case YPPROC_PUSH:
		yppush(rqstp, transp);
		break;

	case YPPROC_PULL:
		yppull(rqstp, transp);
		break;

	case YPPROC_GET:
		ypget(rqstp, transp);
		break;

	default:
		svcerr_noproc(transp);
		break;

	}

	return;
}

/*
 * This increments the global tick counter and tick counters on the time-based
 * functions.  Tick counters associated with the time-based functions latch at
 * their threshold value.  yptimer is a Unix SIGALRM handler, and re-sets the
 * alarm each time it is called.
 */
void
yptimer()
{
	struct timer_action * palarm_action;
	
	tick_counter++;

	for (palarm_action = alarm_actions;
	   palarm_action->ta_action != (PFV) NULL;
	   palarm_action++) {

		if ( (palarm_action->ta_ticks) <
		    palarm_action->ta_threshold) {
			palarm_action->ta_ticks++;
		}
		
	}

	alarm(yptime_quantum);
}

/*
 * This sets the boolean ypdb_xfer_done TRUE.  This is the Unix SIGCHILD
 * handler.
 */
void
ypxfr_proc_exit ()
{
	ypdb_xfer_done = TRUE;
}

/*
 * This tries to get the current map transferred from a peer server to a
 * local temp file.  The actual transfer is accomplished by forking off a
 * map transfer process:  function ypdo_xfr in module ypserv_xfr.c.  The
 * current map's map_xfr_entry will have the transfer process' pid, and
 * the temporary map file name filled in.  
 */
void
ypxfr_map()
{
	struct map_list_item *pmap;
	struct domain_list_item *pdom;
	int pid;

	if (!ypset_current_xfr() )
	    return;				/* Internal error */
	    
	pmap = current_transfer->mx_map;

	if (!pmap) {				/* Map isn't in current list */
		yprelease_current_xfr();
		return;
	}
	
	/* If I am the master peer, I've already got the official copy */
	
	if ((pmap->map_master) &&
	    (!strcmp(pmap->map_master->peer_pname, myhostname) ) ){
		yprelease_current_xfr();
		return;
	}

	pdom = pmap->map_domain;

	if (!pdom->dom_supported) {		/* Domain is unsupported */
		yprelease_current_xfr();
		return;
	}
	
	ypset_current_tmpname();	/* Get a temporary name for the map */
	
	/*
	 * Fork off a map transfer process to do the work, and remember its pid.
	 */
	 
	errno = 0;
	pid = fork();

	if (pid > 0) {
		ypset_current_pid(pid);
	} else if (pid == -1) {
		fprintf(stderr,
		    "ypserv:  ypxfr_map:  fork failure. errno = %d.\n", errno);
	} else {
		ypdo_xfr();
	}
}

/*
 * This is called from the mainline when the SIGCHILD handler has noticed
 * that a child process has exited.  Two sorts of child processes may have
 * exited: either a map transfer process (implying that this is a slave
 * server) or a peer notification process, forked off to send "get" messages
 * to all the peer servers.  In this second case, this is the master ypserv.
 * 
 * A map transfer process exits when a map transfer has completed, either
 * correctly or with errors.  We tell whether the transfer succeeded or
 * failed by reaping the exit status.  In the first (success) case, the map
 * is renamed from its temporary name to its true name, and, if it is a
 * special case map, the appropriate handler function is called.  In the
 * second, failure case, a decision is made on the basis of failure type to
 * either requeue the transfer, or to give up.  In the success case, the
 * transferred map is renamed to its true name.
 * 
 * A peer notification process exits when it has sent a "get" message to all
 * reachable peers.  In this case, we have no interest in any exit status -
 * we just want to reap the exit status to get rid of the zombie.
 */
void
ypmap_xfer_done()
{
	char temp_base[MAXNAMLEN + 1];
	struct map_list_item *pmap;
	int case_index;
	int pid;
	union wait wait_status;
	datum key, val;
	int termsig, retcode;		/* To make debugging easier */
	int error;

	ypdb_xfer_done = FALSE;		/* Prevent further calls for this
					 *   completion */
	pid = 0;

   	if (current_transfer == (struct map_xfr_entry *) NULL) {

		/*
		 * No transfer was in progress - the child must have been a
		 * peer notification processes.  Reap all exit statuses
		 * without looking at them to make sure zombies don't
		 * accumulate at the master server.
		 */
			 
		while (TRUE) {
			pid = wait3(&wait_status, WNOHANG, NULL);

			if (pid == 0) {
				break;
			} else if (pid == -1) {
				break;
			}
		}

		return;
	}

   	if (current_transfer->mx_xfr_pid == 0) {
		
		/*
		 * This represents an error condition.  We really shouldn't
		 * believe we have an active transfer in progress, but not
		 * know the pid of the transfer process.  Recover by
		 * throwing away the exit statuses, and by clearing the
		 * current transfer item.
		 */
	
		while (TRUE) {
			pid = wait3(&wait_status, WNOHANG, NULL);

			if (pid == 0) {
				break;
			} else if (pid == -1) {
				break;
			}
		}

		yprelease_current_xfr();
		return;
	}

	while (pid != current_transfer->mx_xfr_pid) {
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

		/* Success branch */

		/*
		 * If the map pointer in the current transfer entry has been
		 * NULLed out while the transfer was going on, we are going to
		 * bag the rest of this.  Throw away the temp map files and
		 * the current transfer entry, and return from this point.
		 */
		     
		if (current_transfer->mx_map == (struct map_list_item *) NULL) {
			ypdel_mapfiles(current_transfer->mx_temp_path);
			yprelease_current_xfr();
			return;
		}
	
		/*
		 * Temporarily move the newly transferred map into the target
		 * domain, and make a temporary maplist entry for it, using the
		 * temporary name.  This allows us to use the available
		 * primitives for setting the current map and retrieving the
		 * order number.  If the map is well-formed, we will rename the
		 * map data base files; otherwise we will delete them.  In
		 * either case, we will delete the temporary map list entry.
		 */

		ypmkfilename(current_transfer->mx_map->map_domain->dom_name,
		     current_transfer->mx_temp_name, temp_base);

		if (!yprename_map(current_transfer->mx_temp_path, temp_base) ) {

			/*
			 * The rename has failed;  cleanup and return from
			 * this point.
			 */
			 
			ypdel_mapfiles(current_transfer->mx_temp_path);
			yprelease_current_xfr();
			return;
		 }

		strcpy(current_transfer->mx_temp_path, temp_base);
		 
		if (!ypadd_one_map(current_transfer->mx_temp_name,
		    current_transfer->mx_map->map_domain, NULL) ) {

			/*
			 * Couldn't add the new temp map;  cleanup and return
			 * from this point.
			 */
			 
			ypdel_mapfiles(current_transfer->mx_temp_path);
			yprelease_current_xfr();
			return;
		}

		pmap = yppoint_at_map(current_transfer->mx_temp_name,
		    current_transfer->mx_map->map_domain);
		pmap->map_exists = TRUE;
		
		/*
		 * Test to see if the new map has a retrievable order number.
		 * If it does, overwrite the order number held in the maplist
		 * entry with the new one, and rename the map.
		 */

		 if (ypget_map_order(pmap) ) {

			 /*
			  * Do special-case processing on the map ypdomains in
			  * domain yp_private.  Make sure that there is an
			  * entry for yp_private itself, so the world doesn't
			  * fall apart.  If there's none, cleanup and return
			  * from this point.
			  */

			 if (!strcmp(current_transfer->mx_map->map_name,
			     YPDOMAINS) &&
			     !strcmp(
			         current_transfer->mx_map->map_domain->dom_name,
			         ypprivate_domain_name) ) {

				if (!ypset_current_map(pmap->map_name,
				    pmap->map_domain->dom_name, &error) ) {
					ypdel_mapfiles(
					    current_transfer->mx_temp_path);
					yprelease_current_xfr();
					ypdel_one_map(pmap);
					return;
				}

				key.dptr = ypprivate_domain_name;
				key.dsize = sizeof(YPPRIVATE_DOMAIN_NAME) -1;
				val = fetch(key);

				if (val.dptr == (char *) NULL) {
					ypdel_mapfiles(
					    current_transfer->mx_temp_path);
					yprelease_current_xfr();
					ypdel_one_map(pmap);
					return;
				}				 
			 }

			 /*
			  * The map is well-formed.  Make the base file name
			  * be the true map name, and try to do the rename.
			  */
			  
			 ypmkfilename(
			     current_transfer->mx_map->map_domain->dom_name,
			     current_transfer->mx_map->map_name, temp_base);
			     
			 if (yprename_map(current_transfer->mx_temp_path,
			     temp_base) ) {

				/*
				 * This is the go path.  Copy the order number
				 * from the temp maplist entry to the real
				 * maplist entry, mark the map as existing
				 * and supported, and delete the temporary
				 * maplist entry and the current transfer entry.
				 * Notice that after we ditch the temporary map,
				 * we will reuse the map pointer (pmap) for the
				 * real map.
				 */

				current_transfer->mx_map->map_order =
				    pmap->map_order;
				ypdel_one_map(pmap);
				pmap = current_transfer->mx_map;
				yprelease_current_xfr();
				pmap->map_exists = TRUE;
				pmap->map_supported = TRUE;
				
				/*
		 		 * If the map is in the list of special cases,
				 * call the special case handler.
		 		 */
		 
				if ( (case_index = ypspecial_casep(
				    pmap->map_name, map_special_cases) ) >= 0) {
					special_map_handlers[case_index](pmap);
				}

			 } else {

				/*
				 * The rename from temp name to real name has
				 * failed.  Clean up the temp map files, the
				 * current transfer entry, and the temp maplist
				 * entry.
				 */
				 
				ypdel_mapfiles(current_transfer->mx_temp_path);
				yprelease_current_xfr();
				ypdel_one_map(pmap);
			 }
			 
		 } else {

			/*
			 * The new map is ill-formed - we can't retrieve the
			 * order number from the map.  Do the same cleanup
			 * needed when the final rename failed.
			 */
			
			ypdel_mapfiles(current_transfer->mx_temp_path);
			yprelease_current_xfr();
			ypdel_one_map(pmap);
		 }

	} else {

		/*
		 * Either the map transfer process got clobbered and died, or
		 * it was unsuccessful, indicated the condition through its
		 * exit code, and left under its own volition.  Based on the
		 * the error code, decide whether to release or to requeue the
		 * current map transfer entry.
		 *
		 * Implementation note:  This is a first guess as to which
		 * conditions should result in requeue, and which in giving
		 * up (release).
		 */
		 
		ypdel_mapfiles(current_transfer->mx_temp_path);

		if (termsig) {
				yprequeue_current_xfr();
		} else {
			
			switch (retcode) {

			case YPXFR_EXIT_MAP:
			case YPXFR_EXIT_OLD:
			case YPXFR_EXIT_DOMAIN:
			case YPXFR_EXIT_SKEW:
			case YPXFR_EXIT_FORM:
			case YPXFR_EXIT_PEER: {
			case YPXFR_EXIT_RPC:
				yprelease_current_xfr();
				break;
			}

			case YPXFR_EXIT_DBM:
			case YPXFR_EXIT_FILE:
			case YPXFR_EXIT_RSRC:
			case YPXFR_EXIT_ERR: {
				yprequeue_current_xfr();
				break;
			}

			default: {
				yprelease_current_xfr();
				break;
			}

			}
		}
	}
}

/*
 * This flushes output to stderr, then aborts the server process to leave a
 * core dump.
 */
static void
ypexit()
{
	(void) fflush(stderr);
	(void) abort();
}
