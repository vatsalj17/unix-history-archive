#include	<libc.h>
#include	<fio.h>
#include	"sym.h"
#include	"worm.h"
#include	<sys/types.h>
#include	<sys/stat.h>

char *prefix = "";
int dflag = 0;
int quiet = 0;

main(argc, argv)
	char **argv;
{
	Superblock s;
	char *e;
	int c;
	char *dev = "/dev/worm0";
	extern char *optarg;
	extern int optind;

	while((c = getopt(argc, argv, "df:p:s")) != -1)
		switch(c)
		{
		case 'd':	dflag++; break;
		case 'f':	dev = optarg; break;
		case 'p':	prefix = optarg; break;
		case 's':	quiet = 1; break;
		case '?':	usage();
		}
	if(optind >= argc)
		usage();
	dev = mapdev(dev);
	if((s.fd = open(dev, 0)) < 0){
		perror(dev);
		exit(1);
	}
	if(e = openinode(&s, DO_INODE|SPIN_DOWN)){
		fprint(2, "%s: %s\n", dev, e);
		exit(1);
	}
	if(strcmp(s.vol_id, argv[optind])){
		fprint(2, "vol_id mismatch: wanted %s, got %s\n", argv[optind], s.vol_id);
		exit(1);
	}
	optind++;
	c = 0;
	if(optind >= argc){
		while(e = Frdline(0))
			if(pr(&s, e))
				c = 1;
	} else {
		while(optind < argc)
			if(pr(&s, argv[optind++]))
				c = 1;
	}
	exit(c);
}

usage()
{
	fprint(2, "Usage: worm read [-fdevice] [-pprefix] [-d] vol_id [files ...]\n");
	exit(1);
}

pr(s, name)
	Superblock *s;
	char *name;
{
	register Inode *i;
	char b[63*1024L];
	register long len, n;
	long nb;
	int fd;
	char buf[4096];

	if((i = inodeof(name)) == 0){
		fprint(2, "%s not found\n", name);
		return(1);
	}
	sprint(buf, "%s%s", prefix, name);
	name = buf;
	if((fd = create(name, i->mode)) < 0){
		if(dflag){
			createdirs(name);
			fd = create(name, i->mode);
		}
		if(fd < 0){
			perror(name);
			return(1);
		}
	}
	if(fd == 0)	/* a directory */
		return(0);
	nb = sizeof b / s->blocksize;
	Seek(s, i->block);
	for(n = i->nbytes, len = nb*s->blocksize; n > 0;){
		if(len > n){
			len = n;
			nb = (len+s->blocksize-1)/s->blocksize;
		}
		if(Read(s, b, nb)){
			fprint(2, "while writing %s: ", name);
			perror("read");
			exit(1);
		}
		if(write(fd, b, (int)len) != len){
			fprint(2, "while writing %s: ", name);
			perror("write");
			exit(1);
		}
		n -= len;
	}
	close(fd);
	return(0);
}

createdirs(s)
	char *s;
{
	char *ls, *ss;

	for(ls = s; *ls == '/'; ls++)
		;
	for(; *ls && (ss = strchr(ls, '/')); ls = ss+1){
		*ss = 0;
		if(access(s, 0) < 0){
			if(mkdir(s, 0777) < 0){
				perror(s);
				return;
			} else if(!quiet)
				fprint(2, "created %s\n", s);
		}
		*ss = '/';
	}
}

create(name, mode)
	char *name;
{
	if((mode&S_IFMT) == S_IFDIR){
		if(access(name, 0) >= 0){
			if(chmod(name, mode) < 0){
				perror(name);
				return(-1);
			}
		} else {
			if(mkdir(name, mode) < 0){
				perror(name);
				return(-1);
			}
		}
		return(0);
	} else
		return(creat(name, mode));
}
