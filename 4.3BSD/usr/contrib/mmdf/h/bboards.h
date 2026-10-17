/* bboards.h - definition of a BBoard structure */

#define	BBOARDS	"bboards"	/* name in /etc/passwd */
#define	DISTADR	"dist-"		/* prefix for distribution addresses */
#define	BBMODE	0644		/* default BBoards mode */

struct bboard {
    char   *bb_name;		/* name of the bboard */
    char  **bb_aka;		/* aliases for the bboards */

    char   *bb_file;		/* file it resides in */
    char   *bb_archive;		/* file where archives reside */
    char   *bb_info;		/* file where maxima resides */
    char   *bb_map;		/* file where binary map resides */

    char   *bb_passwd;		/* password for it */

    char  **bb_leader;		/* list of local leaders */

    char   *bb_addr;		/* network address */
    char   *bb_request;		/* network address for requests */
    char   *bb_relay;		/* host acting as relay in local domain */
    char  **bb_dist;		/* distribution list */

    unsigned int    bb_flags;	/* various flags */
#define	BB_NULL	0x0000
#define	BB_ARCH	0x0007		/* archive policy */
#define   BB_ASAV	0x0001	/*   save in archives/ directory */
#define	  BB_AREM	0x0002	/*   remove without saving */
#define	  BB_INVIS	0x0010	/* invisible to bbc */

    unsigned int    bb_count;	/* unassigned */
    unsigned int    bb_maxima;	/* highest BBoard-Id in it */
    char   *bb_date;		/* date that maxima was written */

    struct bboard *bb_next;	/* unassigned */
    struct bboard *bb_link;	/* unassigned */
};

int     setbbent (), endbbent (), setbbfile (), ldrbb (), ldrchk (),
	getbbdist ();
char   *getbberr ();
struct bboard  *getbbent (), *getbbnam (), *getbbaka (), *getbbcpy();
