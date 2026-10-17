#include "filsys.h"
#define NMOUNT 20

struct mtab {
	char fsys[10];
	char node[10];
	int  ro_flg;
	int  date[2];
	}mtab[NMOUNT];

char	*flg[] {
	"read/write",
	"read only"
	};

struct filsys super;
main(argc,argv)
char **argv;
{
	int rec, p, dev;
	register struct mtab *mp;
	register char *np;
	char	*ctime();


	rec = open("/etc/mnttab",0);
	read(rec, mtab, sizeof mtab);
	if(argc == 1) {
		for(mp = mtab; mp < &mtab[NMOUNT]; mp++) {
			if(mp->fsys[0]) {
				printf("%s on /dev/%s %s on %s",
				mp->node, mp->fsys,
				flg[mp->ro_flg], ctime(mp->date));
			}
		}
		return;
	}
/* check for proper arguments */

	if(argc == 2) {
		printf("USAGE: mount [device] [name]\n");
		exit(1);
	}
	if(*argv[2] != '/') {
		printf("USAGE: preceed argument with / such as : /%s\n",argv[2]);
		exit(1);
	}
	if((dev = open(argv[1],0)) <1) {
		printf("Can't Open %s\n",argv[1]);
		exit(1);
	}
	seek(dev,1,3);
	read(dev,&super,512);
	if(cmp(argv[2],super.s_fname)) 
		printf("WARNING!! - mounting: <%.6s> as <%.6s>\n",
			super.s_fname, argv[2]);
	close(dev);

	if(mount(argv[1],argv[2], argc > 3) <0) {
		perror("mount");
		exit(1);
	}
	np = argv[1];
	while(*np++);
	np--;
	while(*--np == '/')
		*np = '\0';
	while(np > argv[1] && *--np != '/');
	if(*np == '/') np++;

	argv[1] = np;
	for(mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if(mp->fsys[0] == 0) {
			for(np = mp->fsys; np < &mp->fsys[10];)
				if((*np++ = *argv[1]++ ) == 0) argv[1]--;
			for(np = mp->node; np < &mp->node[10];)
				if((*np++ = *argv[2]++) == 0) argv[2]--;
			time(mp->date);
			mp->ro_flg = argc - 3;
			mp = &mtab[NMOUNT];
			while((--mp)->fsys[0] == 0);
			rec = creat("/etc/mnttab",0644);
			write(rec, mtab, (mp-mtab+1)*sizeof mtab[0]);
			exit(0);
		}
	}
}
cmp(s1, s2)
char	*s1, *s2;
{
	int	i;
	*s1++;
	for(i=0;i<5;i++) {
		if(*s1 == *s2 ) {
			if(*s1 == '\0') return(0);
			s1++; s2++;
			continue;
		}else return(1);
	}
	return 0;
}
