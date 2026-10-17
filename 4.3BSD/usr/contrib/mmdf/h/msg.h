struct msg_struct                /* Structure for sorting files        */
{
    time_t  mg_time;             /* Creation time of message           */
#define LARGESIZE 100000         /* threshold for large message        */
#ifdef LARGESIZE
    char    mg_large;            /* message is over-sized              */
#endif
    char    mg_stat;             /* options                            */
    char    mg_mname[MSGNSIZE];  /* Filename                           */
    char    mg_null;             /* Null for terminating string        */
};
typedef struct msg_struct Msg;

/** The following bits are use in mg_stat **/
#define ADR_CITE   0001           /* When a message is determined to be */
				  /* undeliverable to some addressee(s),*/
				  /* MMDF notifies the sender (if a     */
				  /* return address was given) and an   */
				  /* entire copy of the message is      */
				  /* included in the return, unless this*/
				  /* flag is set, in which case only    */
				  /* a "citation" is returned.          */
#define	ADR_NORET  0002		  /* Do NOT return to sender on error   */
#define	ADR_NOWARN 0004		  /* Do NOT send non-delivery warnings  */
#define	ADR_WARNED 0010		  /* Warning has been sent 		*/

#define msg_cite(val)   (val&ADR_CITE)
#define msg_noret(val)	(val&ADR_NORET)
#define msg_nowarn(val)	(val&ADR_NOWARN)
#define msg_warned(val)	(val&ADR_WARNED)
