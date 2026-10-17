/*	@(#)ypsym.h 1.1 85/05/30 SMI	*/

/*
 * ypsym.h 
 * This contains symbol and structure definitions for
 * modules in the yellow pages server.
 */

#include <dbm.h>			/* Pull this in first */
extern void dbmclose();			/* Refer to dbm routine not in dbm.h */
#ifdef NULL
#undef NULL				/* Remove dbm.h's definition of NULL */
#endif
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

#define __YP_PATH_PREFIX "/etc/yp"
#define YPDBPATH_LENGTH sizeof(__YP_PATH_PREFIX)
#define YPPRIVATE_DOMAIN_NAME "yp_private"
#define YPDOMAINS "ypdomains"
#define YPSERVERS "ypservers"
#define YPMAPS "ypmaps"
#define YPHOSTS_BYNAME "hosts.byname"
#define ORDER_KEY "YP_LAST_MODIFIED"
#define ORDER_KEY_LENGTH (sizeof(ORDER_KEY) - 1)
#define YPTEMPDIRECTORY __YP_PATH_PREFIX
#define YPTEMPNAME_PREFIX "YP_MAP_TRANSFER_"
#define MAX_ASCII_ORDER_NUMBER_LENGTH 10

typedef void (*PFV)();
typedef int (*PFI)();
typedef unsigned int (*PFU)();
typedef long int (*PFLI)();
typedef unsigned long int (*PFULI)();
typedef short int (*PFSI)();
typedef unsigned short int (*PFUSI)();

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef NULL
#undef NULL
#define NULL 0
#endif


#define YPINTERTRY_TIME 10		/* Seconds between tries for peer bind */
#define YPTOTAL_TIME 30			/* Total seconds until timeout */
#define YPPING_TIME 20			/* Total seconds until timeout on ping */
#define YPTIME_QUANTUM 60		/* Time between alarm clock
					 *   interrupts in seconds. */
#define YPPEER_PINGDELTA (10) 		/* Ticks till peer pingable*/
#define YPMAP_PINGDELTA  (10) 		/* Ticks till map pingable */
#define YPNOPORT ((unsigned short) 0)	/* Out-of-range port value */

/* Map transfer process exit/error codes. */

#define YPXFR_EXIT_SUCC 0		/* Success.  The map exists under its
					 *   temporary name. */
#define YPXFR_EXIT_PEER 1		/* Couldn't bind to peer, or peer did
					 *   not respond. */
#define YPXFR_EXIT_DBM 2		/* dbm error on temp map file */
#define YPXFR_EXIT_FILE 3		/* File I/O error */
#define YPXFR_EXIT_SKEW 4		/* Order number in file not the same as
					 *   the order number reported by peer;
					 *   or, order number reported by peer
					 *   changed over the transfer, or peer
					 *   sent more than one order number
					 */
#define YPXFR_EXIT_RSRC 5		/* Local or remote resource allocation
					 *   failure */
#define YPXFR_EXIT_RPC 6		/* rpc failure */
#define YPXFR_EXIT_FORM 7		/* Ill-formed map:  no order number */
#define YPXFR_EXIT_ERR 8		/* Internal error */
#define YPXFR_EXIT_DOMAIN 9		/* Peer does not serve named domain */
#define YPXFR_EXIT_MAP 10		/* Peer does not serve named map */
#define YPXFR_EXIT_OLD 11		/* Peer's version of the map is less
					 *   current than our own.  */


/* yp structure definitions */

struct domain_list_item {
	struct domain_list_item *dom_pnext;
	char dom_name[YPMAXDOMAIN + 1];
	bool dom_supported;
	bool dom_in_new_map;
	struct map_list_item *dom_pmaplist;
	struct peer_list_item *dom_ppeerlist;
};

struct map_list_item {
	struct map_list_item *map_pnext;
	char map_name[YPMAXMAP + 1];
	struct domain_list_item *map_domain;
	bool map_exists;
	bool map_supported;
	bool map_in_new_map;
	unsigned long map_order;
	unsigned long map_last_polled;
	struct peer_list_item *map_master;
	struct peer_list_item *map_alternate;
};

struct peer_list_item {
	struct peer_list_item *peer_pnext;
	char *peer_pname;
	struct in_addr peer_addr;		/* In network order */
	unsigned short peer_port;		/* In network order */
	bool peer_reachable;
	bool peer_in_new_map;
	unsigned long peer_last_polled;
};

struct map_xfr_entry {
	struct map_xfr_entry *mx_pnext;
	int mx_xfr_pid;
	struct map_list_item *mx_map;
	char mx_temp_name[YPMAXMAP + 1];
	char mx_temp_path[MAXNAMLEN + 1];
};

struct timer_action {
	PFV ta_action;			/* Function to call */
	int ta_ticks;			/* Ticks of length YPTIME_QUANTUM
					 *   counted since last called */
	int ta_threshold;		/* Tick threshold; call when reached */
};

/* External refs to cells and functions outside of the yp */

extern long errno;
extern int svc_fds;

/* External refs to yp server data structures */

extern bool ypdb_xfer_done;
extern unsigned long tick_counter;
extern struct map_xfr_entry *map_xfr_list;
extern struct map_xfr_entry *current_transfer;
extern bool ypinitialization_done;
extern char ypdbpath[];
extern int ypdbpath_length;
extern char current_map[];
extern char peer_transport[];
extern char order_key[];
extern struct timeval ypintertry;
extern struct timeval yptimeout;
extern char *domain_special_cases[];
extern char *map_special_cases[];
extern char myhostname[256];
extern PFV special_map_handlers[];
extern struct dom_binding xfr_binding;
extern struct peer_list_item *xfr_peer;
extern struct domain_list_item *pingpeer_curr_domain;
extern struct domain_list_item *pingmap_curr_domain;
extern char ypprivate_domain_name[];
extern char ypdomains[];

/* External refs to yp client-common functions */

extern int _ypclnt_domatch();
extern int _ypclnt_dofirst();
extern int _ypclnt_donext();

/* External refs to yp server-only functions */

extern void ypdispatch();
extern void ypinit();
extern void yptimer();
extern void ypdomain();
extern void ypmatch();
extern void ypfirst();
extern void ypnext();
extern void yppoll();
extern void yppull();
extern void yppush();
extern void ypget();

extern bool ypget_all_domains ();
extern void ypget_supported_domains();
extern void ypadd_one_domain();
extern void ypdel_one_domain();
extern struct domain_list_item *ypcheck_domain();
extern struct domain_list_item *yppoint_at_domain();
extern struct domain_list_item *yppoint_at_first_domain();
extern struct domain_list_item *yppoint_at_next_domain();
extern struct peer_list_item *yppoint_at_peerlist();
extern struct map_list_item *yppoint_at_maplist();
extern char *yppoint_at_domname();
extern void ypdel_domain_lists();

extern struct map_list_item *ypcheck_map();
extern bool ypcheck_map_existence();
extern void ypget_all_maps();
extern void ypget_dom_all_maps();
extern void ypget_supported_maps();
extern void ypget_dom_supported_maps();
extern bool ypadd_one_map();
extern void ypdel_one_map();
extern struct map_list_item *yppoint_at_map();
extern bool ypadd_named_map();
extern void ypping_eligible_map();
extern void ypping_map();
extern char *yppoint_at_mapname();
extern struct map_list_item *yppoint_at_next_map();
extern bool ypget_map_order();
extern struct peer_list_item *ypget_map_master();
extern struct peer_list_item *yppoint_at_map_master();
extern struct peer_list_item *yppoint_at_map_alternate();
extern bool ypfind_alternate();
extern bool ypreturn_map_order();
extern bool ypset_current_map();
extern void ypnew_ypdomains();
extern void ypnew_ypservers();
extern void ypnew_ypmaps();
extern void ypnew_hosts();

extern void ypbuild_peer_lists();
extern void ypbld_dom_peerlist();
extern bool ypget_peer_addr();
extern bool ypadd_one_peer();
extern void ypdel_one_peer();
extern struct peer_list_item *yppoint_at_peer();
extern struct peer_list_item *ypnext_peer();
extern struct in_addr ypreturn_peer_addr();
extern void ypget_all_peer_addrs();
extern void ypreset_all_peer_addrs();
extern void ypping_eligible_peer();
extern void ypping_peer();
extern void ypping_all_peers();
extern bool ypbind_to_named_server();
extern void ypsend_getreq();

extern bool ypadd_xfr();
extern void ypdel_xfr();
extern struct map_xfr_entry *yppoint_at_xfr();
extern void ypenqueue_xfr();
extern bool ypset_current_xfr();
extern void yprelease_current_xfr();
extern void yprequeue_current_xfr();
extern bool ypset_current_pid();
extern bool ypset_current_tmpname();
extern void ypdo_xfr();

extern void ypmap_xfer_done();
extern void ypxfr_proc_exit();
extern void ypmkfilename();
extern void ypxfr_map();
extern int ypspecial_casep();
extern void ypmk_tmpname();
extern bool yprename_map();
extern void ypdel_mapfiles();
extern bool ypset_xfr_peer();
extern void ypclr_xfr_peer();
extern int yppoll_for_order_number();
