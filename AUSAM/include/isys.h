#
/*
 *	ISYS.H - parameter and structure definitions for the
 *	Information System
 *
 *	Copyright (c) 1977, the Children's Museum
 *
 *	This software is furnished to the purchaser under a license
 *	for use on a single computer system and may be copied (with
 *	inclusion of this copyright notice) only for use on that
 *	system, except as may otherwise be provided in the License
 *	or elsewhere in writing by the Children's Museum.
 *
 *	This software is warranted to conform to the Software
 *	Product Description for this product, SPD 11-77-1. No
 *	other warranties are provided, express or implied.
 *
 *	The information in this document is subject to change
 *	without notice.
 */

/* adjustable parameters */
#define MODES 0666		/* all files are created with this mode */
#define DELETED "!deleted!"	/* reserved descriptor for deleted items */
#define NDESCS 256		/* max descriptors per item - used by get() */
#define BUFSIZ 8192		/* max item size for enter command and get() */

/* critical design parameters - should not be changed */
#define DICTIONARY 0
#define OVERFLOW 1
#define TEMPORARY 2
#define EMPTY 250
#define MAXOVSZ 83
#define PGSIZ 512
#define MAXDESC	64	/* length of longest permitted descriptor;
				   6 + MAXDESC + LTHRES must be < 128 */
#define LTHRES 20		/* LILF threshold */
#define PTRSIZE 2
#define	MAXSIZ	127	/* maximum number of longs on a range descriptor file page */
#define	EQUALS	1	/* range descriptor evaluation conditions */
#define	LESSTHAN	2
#define	GREATER	4

struct {
	char minor;
	char major;
	int inumber;
	int flags;
	char nlinks;
	char uid;
	char gid;
	char size0;
	int size1;
	int addr[8];
	int actime[2];
	int modtime[2];
} sbuf;

struct filstruct {
	int fildes;
	int filenum;
	char *filename;
} dictionary, overflow, lilf, lock, tempfile, ctrlfile, *fstructp,
datafile, descfile, descindex, rdesc, rindex;

struct dpage {
	int page_number;
	int space_left;
	int physno;
	char modified;
	char page_type;
	int end_ptr;
	int dictsize;
	int space[250];
} incore,newpage;

struct hitlist {
	int fildes;
	int nleft;
	char *nextp;
	char buff[512];
	char hitfname[16];	/* hitlist filename */
};

struct cbt {
	int logblock;
	int mask;
	int physblock;
};

struct {
	long nextdno;
	int lastrdesc;
	int lastrindex;
	int lastlilf;
	int dictsize;
	struct cbt cbtbl[MAXOVSZ];
	int pad1;
} control;

struct istruct{
	char entry_length;
	char il_count;
};

struct {
	int pad2;
	long desc_no;
	char descr[2];
};

struct {
	int integer;
};

struct {
	int fildes;
	int nleft;
	char *nextp;
};

int debflg;
int tempfct;
long i_add_desc();
long time();
long hitcount();
struct hitlist *and();
struct hitlist *andnot();
struct hitlist *or();
long extract();
struct istruct *find_desc();
char *stdlilf;
char *stdrdesc;
extern struct hitlist *allocerr;
char *stdrindex;
int ressize;
char *lilfname;
char tempfname[14];

struct item_head {
	int size;
	char *offset;
	long mod_time;
} header;

struct datum {
	struct item_head hedr;
	long descbuf[NDESCS+2];
} curitem;


struct {
	int spaceleft;
	int nextpg;
	long data[MAXSIZ];
	long pad[6];
} rdescbuf;

struct {
	long start;
	long stop;
	int page;
} indexbuf;

long atol();

struct {
	long value;
	int condition;
} range;

struct hitlist err_return;
#define		NULLQUERY	&err_return

extern struct hitlist *supermk();

#define		SILENT		1
#define		VERBOSE		0		/* for supermk */
