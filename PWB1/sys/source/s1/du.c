#include "stdio.h"
#include "stat.h"
#define EQ(x,y)	(strcmp(x,y)==0)
#define ML	100

struct statb Statb;
char	path[256];
int	Aflag = 0,
	Sflag = 0,
	Noarg = 0;
struct {
	int	dev,
		ino;
} ml[ML];

main(argc, argv)
char **argv;
{
	register	i = 1;
	register unsigned blocks = 0;

	if(EQ(argv[i], "-s")) {
		++i;
		++Sflag;
	} else if(EQ(argv[i], "-a")) {
		++i;
		++Aflag;
	}
	if(i == argc)
		++Noarg;

	do {
		strcpy(path, Noarg? ".": argv[i]);
		blocks = descend(path);
		if(Sflag)
			printf("%l	%s\n", blocks, path);
	} while(++i < argc);

	exit(0);
}

descend(name)
char *name;
{
	int	dir = 0, /* open directory */
		offset,
		dsize,
		entries,
		dirsize;
	struct dir_entry {
		int	dinode;
		char	dname[14];
	} dentry[32];
	register struct dir_entry	*dp;
	register char *c1, *c2;
	int i;
	char *endofname;
	unsigned blocks = 0;

	if(stat(name,&Statb)<0) {
/*		fprintf(stderr, "--bad status < %s >\n", name);*/
		return(0);
	}
	if(Statb.i_nlink > 1 && (Statb.i_mode&IFMT)!=IFDIR) {
		static linked = 0;

		if(linked >= ML)
			return 0;
		for(i = 0; i <= linked; ++i) {
			if(ml[i].ino==Statb.i_ino && ml[i].dev==Statb.i_dev)
				return 0;
		}
		ml[linked].dev = Statb.i_dev;
		ml[linked].ino = Statb.i_ino;
		++linked;
	}
	blocks = nblock(Statb.i_size0&0377, Statb.i_size1);

	if((Statb.i_mode&IFMT)!=IFDIR) {
		if(Aflag)
			printf("%l	%s\n", blocks, name);
		return(blocks);
	}

	for(c1 = name; *c1; ++c1);
	endofname = c1;
	dirsize = Statb.i_size1;
	for(offset=0 ; offset < dirsize ; offset =+ 512) { /* each block */
		dsize = 512<(dirsize-offset)? 512: (dirsize-offset);
		if(!dir) {
			if((dir=open(name,0))<0) {
/*				fprintf(stderr, "--cannot open < %s >\n",
					name);*/
				return(0);
			}
			if(offset) seek(dir,offset,0);
			if(read(dir, dentry,dsize)<0) {
/*				fprintf(stderr, "--cannot read < %s >\n",
					name);*/
				close(dir);
				return(0);
			}
			if(dir > 10) {
				close(dir);
				dir = 0;
			}
		} else 
			if(read(dir, dentry,dsize)<0) {
/*				fprintf(stderr, "--cannot read < %s >\n",
					name);*/
				close(dir);
				return(0);
			}
		for(dp=dentry, entries=dsize>>4; entries; --entries, ++dp) {
			/* each directory entry */
			if(dp->dinode==0
			|| EQ(dp->dname, ".")
			|| EQ(dp->dname, ".."))
				continue;
			if (dp->dinode == -1)
				continue;
			c1 = endofname;
			*c1++ = '/';
			c2 = dp->dname;
			for(i=0; i<14; ++i)
				if(*c2)
					*c1++ = *c2++;
				else
					break;
			*c1 = '\0';
			if(c1 == endofname) /* ?? */
				return 0;
			blocks += descend(name);
		}
	}
	if(dir)
		close(dir);
	*endofname = '\0';
	if(!Sflag)
		printf("%l	%s\n", blocks, name);
	return(blocks);
}

/* nblock() borrowed from "ls -s" */
nblock(size0, siz)
unsigned size0, siz;
{
	register int n;

	n = ldiv(size0, siz, 512);
	if (siz&0777)
		n++;
	if (n>8)
		n =+ (n+255)/256;
	return(n);
}
