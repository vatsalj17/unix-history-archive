#
/*  Address info stored in queue files                                  */

/*  Every message has an "address list" queued with it.  This file      */
/*  contains the definitions of standard values and formats used in     */
/*  that file.  The primary info is the structure of a single address   */
/*  entry.                                                              */

struct adr_struct                 /* 1 address entry in queued list     */
{
    char    adr_tmp;              /* temp ok on this address            */
#define ADR_AOK   '+'             /*   only address sent, so far        */
#define ADR_CLR   '-'             /*   address is clear to send           */
    char    adr_delv;             /* send to mailbox and/or tty         */
#define ADR_MAIL   'm'            /*   Deliver as mail                  */
#define ADR_TTY    't'            /*   Deliver to tty                   */
#define ADR_BOTH   'b'            /*   Deliver both ways                */
#define ADR_TorM   'e'            /*   either tty or else mail          */
#define ADR_DONE   '*'            /*   No more processing to be done    */
    char   *adr_que;              /* name of output channel queue       */
    char   *adr_host;             /* host string                        */
    char   *adr_local;            /* local-part of address              */
    char   adr_buf[LINESIZE];     /* where to stash the unparsed text   */
    long   adr_pos;		  /* position in address list of this msg */
};

#define ADR_DLOFF   2             /* file offset to adr_delv             */
#define ADR_TMOFF   0             /* file offset to adr_tmp              */
