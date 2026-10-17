/*
 * Print working (current) directory
 */
#include	"stat.h"
#include	"dir.h"

char	dot[]	".";
char	dotdot[]	"..";
char	root[]	"/";
char	name[512];
int	file;
int	off	-1;
struct	statb	x;
struct	dir	y;

main()
{
	register n;

	for (;;) {
		stat(dot, &x);
		if ((file = open(dotdot,0)) < 0) prname();
		do {
			if (read(file, &y, sizeof(y)) < sizeof(y))
				prname();
		} while (y.d_ino != x.i_ino);
		close(file);
		if (y.d_ino == 1)
			ckroot();
		cat();
		chdir(dotdot);
	}
}

ckroot()
{
	register i, n;

	if (stat(y.d_name,&x)<0 || chdir(root)<0 || (file=open(root,0))<0)
		prname();
	i = x.i_dev;
	do {
		if (read(file,&y,sizeof(y)) < sizeof(y))
			prname();
		if (y.d_ino == 0)
			continue;
		if (stat(y.d_name,&x) < 0)
			prname();
	} while (x.i_dev!=i || (x.i_mode&IFMT)!=IFDIR);
	if (strcmp(dot, y.d_name) && strcmp(dotdot, y.d_name))
		cat();
	write(1, root, 1);
	prname();
}

prname()
{
	if (off<0)
		off = 0;
	name[off] = '\n';
	write(1, name, off+1);
	exit(0);
}

cat()
{
	register i, j;

	i = -1;
	while (y.d_name[++i] != 0);
	if ((off+i+2) > 511)
		prname();
	for(j=off+1; j>=0; --j)
		name[j+i+1] = name[j];
	off=i+off+1;
	name[i] = root[0];
	for(--i; i>=0; --i)
		name[i] = y.d_name[i];
}

strcmp(s1, s2)
register char *s1, *s2;
{
	register i;

	while (*s1 == *s2++)
		if (*s1++ == 0)
			return(0);
	return(*s1 - *--s2);
}
