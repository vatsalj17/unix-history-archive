#
#include <local-system>
#include	<passwd.h>

long getoffset(fd)
{
/*
 *	Returns the long length of the file for which "fd"
 *	is a file descriptor. This may then be used as a seek address.
 */

struct
	{
	unsigned	hiword;
	unsigned	loword;
	};

long offset;

struct inode
	{
	int	i_dev;		/* device where inode resides */
	int	i_number;	/* i number, 1-to-1 with device address */
	int	i_mode;
	char	i_nlink;	/* directory entries */
	char	i_uid;		/* owner */
	char	i_gid;		/* group of owner */
	char	i_size0;	/* most significant of size */
	unsigned i_size1;	/* least sig */  /* fix000 */
	int	i_addr[8];	/* device addresses constituting file */
	long	i_atime;		/* fix000 */
	long	i_mtime;		/* fix000 */
	} stat;

fstat(fd,&stat);

offset.loword=stat.i_size1;
offset.hiword=stat.i_size0 & 0377;
return (offset);
}
