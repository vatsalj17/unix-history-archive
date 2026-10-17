#include <stdio.h>
#include <fstab.h>
#include <string.h>
#include <libc.h>
#include <sys/label.h>

/*
 * mount
 */

struct label *atolab();
char *labtoa();

#define	NMOUNT	64

struct mtab mtab[NMOUNT];

main(argc, argv)
int argc;
char **argv;
{
	register struct mtab *mp;
	register struct fstab *fsp;
	char *spec, *file;
	struct label *ceilp = 0;
	struct label ceil;
	int fd;
	int mf;
	int type = 0;
	int flag = 0;
	int mountall = 0;

	nochk(1,0);
	nochk(2,0);
	mf = open(MTAB, 0);
	read(mf, (char *)mtab, sizeof(mtab));
	close(mf);

	if (argc == 1) {
		for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
			if (mp->file[0]) {
				printf("%s on %s", mp->spec, mp->file);
				if (mp->type)
					printf(" type %d", mp->type);
				if((fd = open(mp->file,0)) != -1
				&& labmount(fd, &ceil) != -1)
					printf(" ceil %s", labtoa(&ceil));
				close(fd);
				printf("\n");
			}
		exit(0);
	}
	while(argc>1 && argv[1][0]=='-') {
		switch(argv[1][1]) {
		default:
			usage();
		case 'a':
			if(argc > 2) usage();
			mountall++;
			break;
		case 'r':
			flag++;
			break;
		case 'l':
			if(argc < 3) usage();
			ceilp = atolab(argv[2]);
			if(ceilp == 0) usage();
			argc--, argv++;
			break;
		}
		argc--;
		argv++;
	}
	if (!mountall){
		if(argc < 3) usage();
		spec = argv[1];
		file = argv[2];
		if (argc > 3 && strcmp(argv[3], "-r") == 0) {	/* hack */
			flag = 1;
			argc--;
			argv++;
		}
		if (argc > 3)
			type = atoi(argv[3]);
		if (argc > 4)
			flag = atoi(argv[4]);
		if (mountfs(spec, file, type, flag, ceilp))
			exit(1);
		exit(0);
	}
	if (setfsent() == 0) {
		perror(FSTAB);
		exit(1);
	}
	while ((fsp = getfsent()) != 0) {
		if (strcmp(fsp->fs_file, "/") == 0)
			continue;
		if (fsp->fs_ftype < 0)
			continue;
		if (mountfs(fsp->fs_spec, fsp->fs_file, fsp->fs_ftype,		     fsp->fs_flags, fsp->fs_ceil) == 0)
			babble(fsp);
	}
	endfsent();
	exit(0);
}

/*
 * give a reassuring message about the mount
 * somewhat customised: knows about fs type 0.
 * this is probably silly
 */

babble(fsp)
register struct fstab *fsp;
{
	printf("Mounted %s on %s", fsp->fs_spec, fsp->fs_file);
	if (fsp->fs_ftype != 0) {
		printf(" type %d", fsp->fs_ftype);
		if (fsp->fs_flags)
			printf(" flag %d\n", fsp->fs_flags);
	} else {
		switch (fsp->fs_flags) {
		case 0:
			break;

		case 1:
			printf(" readonly");
			break;

		default:
			printf(" type 0 flag %d", fsp->fs_flags);
			break;
		}
	}
	if(fsp->fs_ceil) 
		printf(" ceiling %s", labtoa(fsp->fs_ceil));
	putchar('\n');
	fflush(stdout);
}

mountfs(spec, name, type, flag, ceilp)
char	*spec, *name;
int type, flag;
struct label *ceilp;
{
	register struct mtab *mp;
	int fd;

	if ((fd = open(spec, 0)) < 0) {
		cant("open %s to mount on %s: ", spec, name, 0);
		return (1);
	}
	if( ceilp ?
	    fmount5(type, fd, name, flag, ceilp):
	    fmount(type, fd, name, flag) < 0) {
		cant("mount %s on %s type %d: ", spec, name, type);
		return (1);
	}
	close(fd);
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if (mp->file[0] == 0) {
			strncpy(mp->file, name, sizeof(mp->file)-1);
			strncpy(mp->spec, spec, sizeof(mp->spec)-1);
			mp->type = type;
			mp = &mtab[NMOUNT];
			while ((--mp)->file[0] == 0);
			if ((fd = creat(MTAB, 0644)) < 0)
				return (0);
			write(fd, (char *)mtab, (mp-mtab+1)*sizeof(struct mtab));
			close(fd);
			return(0);
		}
	}
	return(0);
}

cant(str, a, b, c)
char *str;
char *a, *b;
int c;
{
	extern int errno;
	int serr;

	serr = errno;
	fprintf(stderr, str, a, b, c);
	fflush(stderr);
	errno = serr;
	perror("");
}

usage()
{
	fprintf(stderr, "usage: mount [-a]\n");
	fprintf(stderr,
	  "or: mount [-l label] [-r] special mountpoint [type flag]\n");
	exit(1);
}
