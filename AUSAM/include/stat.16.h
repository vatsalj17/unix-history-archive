/*
 *	definition of buffer returned by new stat for AUSAM
 *
 *	only change is position of "uid" & "nlinks"
 *
 *	N.B. these are in a different position on the disk
 *	     ( see <ino.h> ).
 */

struct 	statbuf
{
	char		sb_minor;
	char		sb_major;
	unsigned	sb_inumber;
	unsigned	sb_flags;
	unsigned	sb_uid;
	char		sb_nlinks;
	char		sb_size0;
	unsigned	sb_size1;
	unsigned	sb_addr[8];
	long		sb_actime;
	long		sb_modtime;
};

#define	IFTYP	060000
#define	IFCHR	020000
#define	IFDIR	040000
#define	IFBLK	060000
#define	IFLOCK	002000
#define	IFFLOK	002000
#define	IFALOK	002010
