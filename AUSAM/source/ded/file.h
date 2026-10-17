extern errno;

#define ENOENT 2        /* no such file, or directory missing */
#define EACCES 13       /* no access allowed */
#define ENOTDIR 20      /* part of pathname is not a directory */

struct inode
 { char minor, major;
    int inumber, flags;
    char nlinks, uid, gid, size0;
    int size1, addr[8], actime[2], modtime[2];
 };
