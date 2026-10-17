#ifndef	BSD4_2

#include <sys/types.h>
#include <ndir.h>

/*
 * read an old stlye directory entry and present it as a new one
 */
#define	ODIRSIZ	14

struct	olddirect {
	ino_t	d_ino;
	char	d_name[ODIRSIZ];
	char	d_pad[10];
};

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register DIR *dirp;
{
	register struct olddirect *dp;
	static struct direct dir;

	for (;;) {
		if (dirp->dd_loc == 0) {
			dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, 
			    DIRBLKSIZ);
			if (dirp->dd_size <= 0)
				return NULL;
		}
		if (dirp->dd_loc >= dirp->dd_size) {
			dirp->dd_loc = 0;
			continue;
		}
		dp = (struct olddirect *)(dirp->dd_buf + dirp->dd_loc);
		dirp->dd_loc += sizeof(struct olddirect);
		if (dp->d_ino == 0)
			continue;
		dir.d_ino = dp->d_ino;
		strncpy(dir.d_name, dp->d_name, ODIRSIZ);
		dir.d_name[ODIRSIZ] = '\0'; /* insure null termination */
		dir.d_namlen = strlen(dir.d_name);
		dir.d_reclen = DIRSIZ(&dir);
		return (&dir);
	}
}
#endif

#ifndef	BSD4_2

DIR *
opendir(name)
register char *name;
{
	DIR dirbuf, *dirp;
	struct stat statb;
	char buf[MAXNAMELEN+1];
	register char *s;

	*(movstrn(name, buf, MAXNAMELEN)) = 0;
	for (s=buf; *s; s++)
		*s &= STRIP;
	if ((dirbuf.dd_fd = open(buf, 0)) < 0)
		return(NULL);
	if (fstat(dirbuf.dd_fd, &statb)!=0 || (statb.st_mode & S_IFMT)!=S_IFDIR){
		close(dirbuf.dd_fd);
		return(NULL);
	}
	dirbuf.dd_loc = 0;
	dirp = (DIR *)shalloc(sizeof(DIR));
	*dirp = dirbuf;
	return(dirp);
}

void
closedir(dirp)
DIR *dirp;
{
	close(dirp->dd_fd);
	shfree((char *)dirp);
}

#endif
