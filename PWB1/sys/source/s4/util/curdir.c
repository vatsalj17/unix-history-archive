static char Sccsid[] "@(#)curdir	1.1";
/*
	current directory.
	Places the full pathname of the current directory in `str'.
	Handles file systems not mounted on a root directory
	via /etc/mtab (see mtab(V)).
	NOTE: PWB systems don't use mtab(V), but they don't mount
	file systems anywhere but on a root directory (so far, at least).

	returns 0 on success
	< 0 on failure.

	Current directory on return:
		success: same as on entry
		failure: UNKNOWN!
*/

# include	<dir.h>
# include	<macros.h>

static char *curdirp;

struct mtab {
	char	m_devstr[6];
	char	m_spcl[32];
	char	m_dir[32];
};

static struct mtab mtab;

curdir(str)
char *str;
{
	register int n;

	copy("/dev//",mtab.m_devstr);
	curdirp = str;
	n = findir(0);
	return(n+chdir(str));
}


# define ADDSLASH	if (flag) *curdirp++ = '/';
# define QUIT		{ close(fd); return(-1); }

findir(flag)
{
	register int fd,inum;
	register char *tp;
	char *slashp;
	int dev, r;
	struct dir entry;
	struct statb s;

	if (stat(".",&s)<0) return(-1);
	if ((inum = s.i_ino) == 1) {
		dev = s.i_dev;
		if ((fd = open("/",0))<0) return(-1);
		if (fstat(fd,&s)<0)
			QUIT;
		if (dev == s.i_dev) {
			*curdirp++ = '/';
			*curdirp = 0;
			close(fd);
			return(0);
		}
		slashp = &entry.d_ino;
		slashp++;
		while (read(fd,&entry,16) == 16) {
			if (entry.d_ino == 0) continue;
			*slashp = '/';
			if (stat(slashp,&s)<0) continue;
			if (s.i_dev != dev) continue;
			if ((s.i_mode&IFMT) != IFDIR) continue;
			for (tp = slashp; *curdirp = (*tp++); curdirp++);
			ADDSLASH;
			*curdirp = 0;
			close(fd);
			return(0);
		}
		if ((fd = open("/etc/mtab", 0))<0) return(-1);
		while (read(fd,mtab.m_spcl,64) == 64) {
			if (stat(&mtab.m_devstr,&s)<0)
				QUIT;
			if (s.i_addr[0] != dev) continue;
			for (tp = mtab.m_dir; *curdirp = *tp++; curdirp++);
			ADDSLASH;
			*curdirp = 0;
			return(0);
		}
		QUIT;
	}
	if ((fd = open("..",0))<0) return(-1);
	for (entry.d_ino = 0; entry.d_ino != inum; )
		if (read(fd,&entry,16) != 16)
			QUIT;
	close(fd);
	if (chdir("..")<0) return(-1);
	if (findir(-1)<0) r = -1;
	else r = 0;
	for (tp = entry.d_name; *curdirp = (*tp++); curdirp++);
	ADDSLASH;
	*curdirp = 0;
	return(r);
}
