#include <stdio.h>

/*
 *      INFORMATION KEPT FOR ALL TABLES
 */
struct tb_struct
{
    char    *tb_name;           /* internal name of table               */
    char    *tb_show;           /* displayable human-oriented string    */
    char    *tb_file;           /* name of file containing table        */
    FILE    *tb_fp;             /* stdio file pointer                   */
    long     tb_pos;            /* position in file                     */
    int	     tb_flags;		/* various bits	(type of table, etc)	*/
};
#define	TB_SRC	000007			/* Source of table data */
#define	TB_FILE		000000		/* Read from file */
#define	TB_DBM		000000		/* Read from DBM database */
#define	TB_NS		000001		/* Read from Nameserver */

#define TB_TYPE 000070			/* what type of question to ask NS */
#define TB_DOMAIN	000010		/* ask the nameserver for a domain */
#define TB_CHANNEL	000020		/* ask the nameserver for a channel */
#define TB_ROOT		000040		/*ask the nameserver for a root domain*/
#define TB_PARTIAL	000100	/* don't ask the nameserver in certain cases */

typedef struct tb_struct    Table;

Table *tb_nm2struct ();

/*****************************************************************/
/*
 *      STRUCTURE USED FOR CACHING  (Version 1, this may change)
 */

struct cache
{
	char    *ca_hostid;     /* host identification */
	int     ca_value;       /* its bad connection value */
	time_t  ca_expire;      /* time at which to expire entry */
	struct  cache *ca_next; /* the next dead host... or NULL */
};

typedef struct cache    Cache;

/*****************************************************************/
/*
 *      STRUCTURE USED FOR ALIAS SOURCES
 */

struct al_source
{
	int	al_flags;
	Table	*al_table;
	struct al_source *al_next;
};
/* For use with al_flags */
#define	AL_TRUSTED	00001	/* this alias file is trustworthy */
#define AL_NOBYPASS	00002	/* this alias file cannot be bypassed */

typedef struct al_source	Alias;

/******************************************************************
 *
 *      STRUCTURES WHICH DESCRIBE CHANNELS
 *
 *  Ch_struct (typedef Chan) contains all of the parametric information
 *  about a channel.  There may be several telephone and/or pobox channels,
 *  but each type is exec'd from a single executable file, so that the
 *  structure provides distinguishing information.  For telephone channels,
 *  Ch_struct has a pointer to an extension structure, called ch_phstruct.
 *  The member is null for other channels.
 */

/*      TELEPHONE-SPECIFIC PARAMETERS
 *
 *  The existing "telephone" channel also is used for low-speed hardwired
 *  machine-machine exchange, through tty ports.
 *
 *  If a phone channel is inactive, by virtue of not having any mail going
 *  from the local machine to the "other side", then we may need to poll
 *  them, periodically, to see if they have any mail to be brought over.
 *  Currently, you are limited to specifying a delay time; ch_poltime
 *  indicates how many 15-minute units of time to wait, since the last
 *  completed exchange, before polling.  NOTE:  you cannot currently
 *  specify time-of-day / day-of-week windows.
 *
 *  In order to keep track of the channel's activity, the files _dstrt,
 *  _ddone, and _pdone are used to note when outbound delivery was last
 *  started, when it was last completed and when the last pickup (inbound
 *  retrieval) was last completed.  The files are zero-length and only
 *  their modification time is of interest.
 *
 *  Ch_access indicates that mail may be retrieved, as well as sent.
 *
 *  ch_script specifies the script file to be followed for creating a
 *  connection to the slave on the other machine.  it usually indicates
 *  what number to call or line to connect to, login sequence, and program
 *  to start running.  it's use is more fully described in the dial
 *  package's documentation.
 */
/**/

/*      DESCRIPTION OF OUTPUT CHANNELS
 *
 *  For active channels, an attempt is made to send queued mail.  For
 *  passive channels, the mail is held until called for.  Some active
 *  channels may be invoked by anyone, others are restricted to the Deliver
 *  daemon (runnable an background).
 *
 *  A channel may have two "names".  One (ch_show) is intended for display
 *  and may contain any characters that will enhance its presentation.
 *  The other (ch_name) is intended for user input and more limited
 *  display.  It should have no special characters (e.g., dash, space) and
 *  should be entirely in lower case.  It must be unique.
 *
 *  A channel services a subset of the entire queue.  It access mail
 *  stored under the sub-queue, specified by ch_queue.  More than one
 *  channel may access a sub-queue, such as having an arpanet channel
 *  and a backup phone channel.
 *
 *  Each channel has an associated table, with the names of hosts on that
 *  channel.  The module, ch_table, handles access to the table and defines
 *  its format.  The local host is usually named in the table.  In order to
 *  capture references to the local machine, we need to know its "official"
 *  name in the table.  ch_lname permits this.
 *
 *  ch_ldomain specifies the RFC822 domain name to use, for this host,
 *  for the channel.
 *
 *  It also may allow some other special handling.  Usually, ch_lname is
 *  the same as the global, locname[], but this is not required.  For
 *  example, if there are several machines providing service to a single
 *  organization, you may want ch_lname to be an internal, local-network
 *  name for the local machine, while locname[] would be the public name
 *  for the organization.
 *
 *  Channels often involve contact with a single, other host, which acts as
 *  a relay for the other machines on a network.  If that is the case, then
 *  the "official" name of that host should be specified (ch_host).  This is
 *  particularly necessary when mail is picked up from (or submitted by)
 *  the relay, so that the Via: field can cite it.
 *
 *  When the channel is passive and a single site acts as a relay for the
 *  entire channel's mail, then there will be a single Unix login
 *  authorized to perform the pickup.  That must be indicated with
 *  ch_login.
 *
 *  If the channel is a telephone-like channel, then the above-described
 *  ch_wait, ch_access, ch_login, must be filled in.
 */
/**/


struct ch_struct
{
    char   *ch_name;              /* specification name of channel      */
    char   *ch_show;              /* pretty-print name for display      */
    Table  *ch_table;             /* pointer to host name table         */
    char   *ch_queue;             /* 'channel' name to enter in queue   */
    int     ch_access;            /* assorted switches                  */
#define DLVRREG    00             /*   Active delivery, by any Deliver  */
#define DLVRBAK    01             /*   Only runnable by daemon          */
#define DLVRPSV    02             /*   Passive wait for pickup          */
#define DLVRDID    04             /* (internal flag) chan was touched   */
				  /*   a flag set by mmdf, sometimes    */
#define DLVRIMM   010             /* may be run immediately (fast/cheap)*/
#define CH_PICK   020             /*   ok to pickup mail                */
#define CH_SEND   040             /*   ok to send mail                  */
#define MQ_INQ   0100             /* queued under this channel          */
    char   *ch_ppath;             /* path to file containing chan pgm   */
    char   *ch_lname;             /* Official host name of local machine */
#define DFLNAME     0             /* use locname as our name on chan    */
#define NOLNAME    -1             /* we are not know on the channel (??)*/
    char   *ch_ldomain;           /* Official domain name of this host  */
    char   *ch_host;              /* name of host doing relay for chan  */
#define NORELAY     0             /* direct receipt, rather than relay  */
    char   *ch_login;             /* valid login name for pickup relay  */
				  /*   ch_access must have PSV bit on   */
				  /*   some PSV chans are direct recip- */
				  /*   ients and not relays             */
#define NOLOGIN     0             /* not a pickup channel               */
    char    ch_poltime;           /* # of 15-minute periods to wait     */
#define ALLPOLL -1                /*  -1 => poll on every wakeup        */
#define NOPOLL   0                /*   0  => no polling allowed         */
    char   *ch_trans;             /* filename of the phone transcript   */
				  /* to use relative to logdfldir       */
#define DEFTRANS 0                /*    0 => use the default trn file   */
    char   *ch_script;            /* pathname to dialing script         */
#define NOSCRIPT 0
    Table  *ch_outsource;         /* list of sources authorized to      */
				  /*   access this channel              */
    Table  *ch_insource;          /* list of sources authorized to      */
				  /*   access this channel              */
#define NOFILTER 0
    Table  *ch_outdest;           /* authorized destinations            */
    Table  *ch_indest;            /* authorized destinations            */
    Table  *ch_known;             /* table of hosts known to hosts on   *
				   * this channel, normally == ch_table */

    char     *ch_confstr;        /* general purpose string              */
    int      ch_apout;            /* default 733 / 822 /same format     */
				  /* for header munging                 */
    char     ch_auth;             /* For authorisation by user control  */
#define CH_FREE        0
#define CH_IN_LOG     01
#define CH_IN_WARN    02
#define CH_IN_BLOCK   04
#define CH_OUT_LOG   010
#define CH_OUT_WARN  020
#define CH_OUT_BLOCK 040
#define CH_HAU      0100        /* authorise on basis of host AND user */
#define CH_DHO      0200

#define CH_IN_AUTH    07
#define CH_OUT_AUTH  070
    Cache   *ch_dead;           /* Current dead hosts */
    time_t  ch_ttl;             /* Number of seconds to keep dead host records */
    char    *ch_logfile;        /* Logfile for this channel */
    int     ch_loglevel;        /* Logging level for this channel */
};


typedef struct ch_struct    Chan; /* make it a "type"                   */

Chan * ch_nm2struct (),           /* chan name -> ptr to its table entry */
     * ch_qu2struct (),           /* queue name -> ptr to its table entry */
     * ch_h2chan ();              /* use host name to find chan name     */

Chan   *ch_h2chan ();          /* which chan name table is host in?  */
