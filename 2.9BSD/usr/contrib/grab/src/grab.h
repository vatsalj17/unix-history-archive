/*
 * grab.h
 *
 * Include file for grab -- contains structure definitions,
 *	external definitions.
 *
 *---------------------------------------------------------------------------
 *
 * $Header:   RCS/grab.h.v  Revision 1.2  83/07/01  19:41:34  donn  Exp$
 * $Log:	RCS/grab.h.v $
 * Revision 1.2  83/07/01  19:41:34  donn
 * Added stuff to support '-i' and '-L' flags.
 * 							Donn
 * 
 */

# include	<stdio.h>



extern int	target_system;	/* Type of file system to be read */
extern int	fsbsiz;		/* File system block size */
extern int	fsys;		/* File descriptor for file system */
extern int	pflag;		/* 1 if file protections are restored */
extern int	xflag;		/* 1 if output goes to stdout */
extern int	vflag;		/* 1 for verifying files during copy */
extern int	iflag;		/* 1 for grabbing by inode */
extern int	lflag;		/* >=1 for listing files in a directory */
extern int	Lflag;		/* 1 for long listings of files */
extern int	tflag;		/* 1 for producing "tar" output */
extern int	nblock;		/* "Tar" output blocking factor */

# define	V7_2BSD		0
# define	V7_4BSD		1
# define	V6		2


/*
 * Default settings for host system.
 * Set PDPV6 for V6, PDP for V7_2BSD and VAX for V7_4BSD.
 */
# ifndef	VAX
# ifndef	PDP

# define	PDPV6		1

# endif
# endif


# ifdef PDPV6

# define	PDP		1

# define	TARGET_DEFAULT	V6
# define	FSB_DEFAULT	512

# define	geteuid()	((getuid() >> 8) & 0xff)
# define	getegid()	((getgid() >> 8) & 0xff)

# define	CHOWN(s,u,g)	chown( (s), ((g)<<8) | ((u)&0xff) )

# define	CONVMODE(m)	((m) & V6_IFMT)

# else

# ifdef		VAX
# define	TARGET_DEFAULT	V7_4BSD
# else
# define	TARGET_DEFAULT	V7_2BSD
# endif

# define	FSB_DEFAULT	1024

# define	CHOWN		chown

# define	CONVMODE(m)	((m)&(target_system==V6?V6_IFMT:V7_IFMT))

# endif PDPV6



/*
 * Generic inode structure.
 * DEFICIENCY: this structure is used to recover the mode word
 *	from a "stat" call...
 */
struct inode
{
	long	i_size;
	short	i_mode;
	short	i_nlink;
	short	i_uid;
	short	i_gid;
	long	i_addr[13];
	long	i_atime;
	long	i_mtime;
};



/*
 * Version 7 disk inode.
 * Kluge for 2.9's 1/2 size inodes added.
 */

# define	V7NADDR		(target_system==V7_2BSD?7:13)

struct v7dinode
{
	short	v7di_mode;		/* mode and type of file */
	short	v7di_nlink;		/* number of links to file */
	short	v7di_uid;		/* owner's user id */
	short	v7di_gid;		/* owner's group id */
	long	v7di_size;		/* number of bytes in file */
	char	v7di_addr[40];		/* disk block addresses */
	long	v7di_atime;		/* time last accessed */
	long	v7di_mtime;		/* time last modified */
	long	v7di_ctime;		/* time created */
};


/*
 * Version 6 disk inode.
 */

# define	V6NADDR		8

struct v6dinode
{
	short	v6di_mode;		/* flags */
	char	v6di_nlink;		/* number of links to file */
	char	v6di_uid;		/* user ID of owner */
	char	v6di_gid;		/* group ID of owner */
	char	v6di_sz0;		/* high byte of 24-bit size */
	short	v6di_sz1;		/* low word of 24-bit size */
	short	v6di_addr[8];		/* block numbers or device numbers */
	long	v6di_atime;		/* time of last access */
	long	v6di_mtime;		/* time of last modification */
};



/*
 * Mode bits.
 * It is assumed that the protection bits 0-11 are the same on all systems.
 */
#define	V7_IFMT		0170000		/* type of file */
#define		V7_IFCHR	0020000	/* character special */
#define		V7_IFBLK	0060000	/* block special */
#define		V7_IFMPC	0030000	/* multiplex char special */
#define		V7_IFMPB	0070000	/* multiplex blk special */
#define		V7_IFDIR	0040000	/* directory */
#define		V7_IFLNK	0120000	/* symbolic link */
#define		V7_IFREG	0100000	/* regular */
#define V6_IFMT		0060000		/* type of file */
#define		V6_IFCHR	0020000	/* character special */
#define		V6_IFBLK	0060000	/* block special */
#define		V6_IFDIR	0040000	/* directory */
#define		V6_IFREG	0000000	/* regular */
#define	V6_ILARGE	0010000		/* large file */
#define	ISUID		0004000		/* setuid */
#define	ISGID		0002000		/* setgid */
#define	ISVTX		0001000		/* sticky */



/*
 * Block map structure.  Used to keep track of a sequential read
 * through the blocks of an inode.
 */
struct bmap
{
	short	b_type;		/* Type of block: NORMAL/HOLE */
	short	b_mode;		/* File mode */
	long	b_size;		/* File size */
	long	b_offset;	/* Offset within file (bytes) */
	int	b_len;		/* Size of buffer */
	int	b_cc;		/* Buffer character count */
	long	b_lbno;		/* Logical block number in file */
	long   *b_iaddr;	/* Block numbers in an inode */
	char   *b_data;		/* Contents of current buffer */
	char   *b_indir[3];	/* Contents of indirect blocks */
	int	b_count[3];	/* Index into an indirect block */
};


/*
 * Block types, buffer types and size.
 */
# define	B_NORMAL	0
# define	B_HOLE		1
# define	B_SMALL		0
# define	B_BIG		1
# ifdef VAX
# define	B_BUFSIZE	128
# else	PDP
# define	B_BUFSIZE	32
# endif


/*
 * Size of a disk sector.
 */
# define	DBSIZE		512


/*
 * Default blocking factor for "tar"-style output, and other "tar" stuff.
 */
# define	TAR_BS_DEFAULT	20
# define	TBLOCK		512
# define	NAMSIZ		100

/*
 * Tar header structure.
 */
union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
	} dbuf;
};



/*
 * CLRBUF(a,b) -- Zero out b bytes starting at a.
 * CPYBUF(a,b,c) -- Copy c bytes into a from b.
 *	These are just experiments to make use of the SOB instructions...
 *	The counts are left at 0, the pointers end up at the end of the data.
 */
# ifdef VAX

# define CLRBUF(a,b)	if (b>0) { do *a++ = '\0'; while ( --b > 0 ); }
# define CPYBUF(a,b,c)	if (c>0) { do *a++ = *b++; while ( --c > 0 ); }

# else

# define CLRBUF(a,b)	if (b>0) { do *a++ = '\0'; while ( --b ); }
# define CPYBUF(a,b,c)	if (c>0) { do *a++ = *b++; while ( --c ); }

# endif



/*
 * Macros for decoding block and inode numbers.
 * DEFICIENCY: This uses on the fact that there are just as many
 *	inodes per block under V6 as under V7 with big blocks.
 */
# define	INOPB		16
# define	fsbtodb(n)	(target_system==V6?(n):(n)<<1)
# define	itod(n)		((long)((((unsigned short)(n)+2*INOPB-1)/INOPB)))
# define	itoo(n)		((int)(((unsigned short)(n)+2*INOPB-1)%INOPB))



/*
 * Form of directory entries.
 */
#define	DIRSIZ	14
#define ROOTINO	(target_system==V6?1:2)

struct	direct
{
	unsigned short	d_ino;
	char		d_name[DIRSIZ];
};


/*
 * Miscellaneous.
 */
# define	min(a,b)	((a)<(b)?(a):(b))
# define	max(a,b)	((a)>(b)?(a):(b))
# define	STDOUT		1
# define	TFILE		STDOUT
