/*
 * Inode structure as it appears on
 * the disk. Not used by the system,
 * but by things like check, df, dump.
 */
struct	inode
{
	int	i_mode;
	char	i_nlink;
#ifndef	AUSAM
	char	i_uid;
	char	i_gid;
#else
	char	i_uidlob;
	char	i_uidhib;
#endif
	char	i_size0;
	unsigned i_size1;		/* fix000 */
	int	i_addr[8];
	long	i_atime;		/* fix000 */
	long	i_mtime;		/* fix000 */
};

/* modes */
#define	IALLOC	0100000
#define	IFMT	060000
#define		IFDIR	040000
#define		IFCHR	020000
#define		IFBLK	060000
#define	ILARG	010000
#define	ISUID	04000
#define	ISGID	02000
#define ISVTX	01000
#define	IREAD	0400
#define	IWRITE	0200
#define	IEXEC	0100
