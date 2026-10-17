/* Fortran get working directory. PLWard 3/20/80 */
/*
 * Fortran callable character function to get working directory 
 * as a string.   PLWard, USGS, Menlo Park, California
 * Modified version of pwd.c.
 */
#include	<stdio.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/dir.h>

char	dot[]	= ".";
char	dotdot[] = "..";
int	file;
struct	stat	d, dd;
struct	direct	dir;

char workdr_(name,lim)
    char name[];
    long lim;
{
	int rdev, rino;
	register i, j;
	int off;

	off = -1;
	for(j=0;j<lim;j++) name[j]=' ';
	stat("/", &d);
	rdev = d.st_dev;
	rino = d.st_ino;
	for (;;) {
		stat(dot, &d);
		if (d.st_ino==rino && d.st_dev==rdev)
			goto out;
		if ((file = open(dotdot,0)) < 0) {
			fprintf(stderr,"workdr: cannot open ..\n");
			return;
		}
		fstat(file, &dd);
		chdir(dotdot);
		if(d.st_dev == dd.st_dev) {
			if(d.st_ino == dd.st_ino)
				goto out;
			do
				if (read(file, (char *)&dir, sizeof(dir)) < sizeof(dir)) {
					fprintf(stderr,"workdr: read error in ..\n");
					return;
				}
			while (dir.d_ino != d.st_ino);
		}
		else do {
				if(read(file, (char *)&dir, sizeof(dir)) < sizeof(dir)) {
					fprintf(stderr,"workdr: read error in ..\n");
					return;
				}
				stat(dir.d_name, &dd);
			} while(dd.st_ino != d.st_ino || dd.st_dev != d.st_dev);
		close(file);
		i = -1;
		while (dir.d_name[++i] != 0);
		if ((off+i+2) >= lim){
			name[off]='\0';
			i=chdir(dir.d_name);
			j=chdir(name);
			if(i!=0 | j!=0) fprintf(stderr,"workdr: unable to change back to working dir\n");
			for(j=0;j<lim;j++)name[j]=' ';
			return;
		}
		for(j=off+1; j>=0; --j)
			name[j+i+1] = name[j];
		off=i+off+1;
		name[i] = '/';
		for(--i; i>=0; --i)
			name[i] = dir.d_name[i];
	}
out:for(j=off+1; j>=0; --j)
		name[j+1] = name[j];
	name[0]='/';
	if (off<0)
		off = 0;
	off=off+1;
	name[off] = '\0';
	chdir(name);
	name[off] = ' ';
}
