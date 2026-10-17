#include "filsys.h"
#define FILE_SYS 1
#define DEV_FROM 2
#define FROM_VOL 3
#define DEV_TO 4
#define TO_VOL 5
char *nblocks NBLOCKS;
char *fs NBLOCKS;
int *ip;
int buf[256*NBLOCKS];
/*

filesystem copy with propagation of volume ID and filesystem name:

  vc??  filesystem /dev/from from_vol /dev/to to_vol

  Example:

  vc88 root /dev/rrp2 pk5 /dev/rrp12 pk12

  vc10 u3 /dev/rrp15 pk1 /dev/rmt0 tp123

  vc10 u5 /dev/rmt0 -  /dev/rrp15 -

In the last example, dashed volume args mean "use label that's there."

From/to devices are printed followed by `?'.
User has 10 seconds to DEL if mistaken!
 */

int devtty;
int from, to;
char *block;
int totape, fromtape;
int first;

struct filsys	superi, supero, *sptr;

main(argc, argv) char **argv; {
int fsi, fso, tvec[2], then[2], file, statbuf[18];
int i;
int	thenloct, nowloct, *localtime();

	devtty = open("/dev/tty", 0);
	if(argc!=6)
		 err("Usage: vc?? fsname /devfrom volfrom /devto volto");

	if((fsi = open(argv[DEV_FROM],0)) < 1) err("cannot open input");
	if(read(fsi, &buf, 512*NBLOCKS) != 512*NBLOCKS)
		err("read error on in dev");
	sptr = &buf[256];
	copy(sptr->s_fname,superi.s_fname);
	copy(sptr->s_fpack,superi.s_fpack);
	superi.s_fsize = sptr->s_fsize;
	superi.s_time = sptr->s_time;
	totape = gmatch(argv[DEV_TO], "*mt?");
	fromtape = gmatch(argv[DEV_FROM], "*mt?");

	if(!equal(argv[FILE_SYS],superi.s_fname)) {
		printf("arg. (%.8s) doesn't agree with from fs. (%.8s)\n",
			argv[FILE_SYS],superi.s_fname);
		ask();
	}
	if(!equal(argv[FROM_VOL],superi.s_fpack)) {
		printf("arg. (%.8s) doesn't agree with from vol.(%.8s)\n",
			argv[FROM_VOL],superi.s_fpack);
		ask();
	}

	if((fso = open(argv[DEV_TO],0)) < 1) err("cannot open output");
	if(read(fso, &buf, 512*NBLOCKS) != 512*NBLOCKS) {
		printf("Read error on out device\n");
		if(totape) {
			ask();
			copy(argv[TO_VOL], supero.s_fpack);
			goto SKIP;
		}
		exit(9);
	}
	sptr = &buf[256];
	copy(sptr->s_fname,supero.s_fname);
	copy(sptr->s_fpack,supero.s_fpack);
	supero.s_fsize = sptr->s_fsize;
	supero.s_time = sptr->s_time;

	if(argv[FROM_VOL][0]=='-') argv[FROM_VOL] = superi.s_fpack;
	if(argv[TO_VOL][0]=='-') argv[TO_VOL] = supero.s_fpack;

	if(later(&supero.s_time,&superi.s_time)) {
		printf("%s less than 36 hours older than %s\n",
			argv[DEV_TO], argv[DEV_FROM]);
		printf("To filesystem dated:  %s", ctime(&supero.s_time));
		ask();
	}
	if(!equal(argv[TO_VOL],supero.s_fpack)) {
		printf("arg.(%.8s) doesn't agree with to vol.(%.8s)\n",
			argv[TO_VOL],supero.s_fpack);
		ask();
		copy(argv[TO_VOL],supero.s_fpack);
	}
	if(superi.s_fsize > supero.s_fsize && !totape) {
		printf("from fs larger than to fs\n");
		ask();
	}
	if(!totape && !equal(superi.s_fname,supero.s_fname)) {
		printf("warning! from fs(%s) differs from to fs(%s)\n",
			superi.s_fname,supero.s_fname);
		ask();
	}


   SKIP:
	close(fsi); close(fso);

	fscopy(argv[DEV_FROM], argv[DEV_TO]);

						/* Post Results */

	if(stat("/etc/log/filesavelog",statbuf) < 0)
		goto create;
	then[0] = statbuf[16]; then[1] = statbuf[17];
	thenloct = (localtime(then))[3];
	time(tvec);
	nowloct = (localtime(tvec))[3];
	close(1);
	if(nowloct < thenloct) {
create:
		file = creat("/etc/log/filesavelog",0644);
		printf("%s\n",ctime(tvec));
		printf("%s(%s)  <%s> WAS COPIED ON %s  <%s>\n",
	superi.s_fname,argv[DEV_FROM],superi.s_fpack,argv[DEV_TO],supero.s_fpack);
	}else {
		file = open("/etc/log/filesavelog",2);
		seek(file,0,2);
		if(nowloct == thenloct) {
			printf("%s(%s)  <%s> WAS COPIED ON %s  <%s>\n",
		superi.s_fname,argv[DEV_FROM],superi.s_fpack,argv[DEV_TO],
		supero.s_fpack);
		}else {
			printf("\n%s\n",ctime(tvec));
			printf("%s(%s)  <%s> WAS COPIED ON %s  <%s>\n",
		superi.s_fname,argv[DEV_FROM],superi.s_fpack,argv[DEV_TO],
		supero.s_fpack);
		}
	}
	close(file);
	exit(0);
}

err(s) char *s; {
	printf("%s\n",s);
	exit(9);
}
int later(now,then) int now[], then[]; {
	if(now[0]+2 > then[0]) return(1); /* 4/3rds == 1 day */
	return(0);
}
equal(s1, s2) char *s1, *s2; {
register int i;

	for(i=0;i<=5;++i) {
		if(*s1 == *s2) {;
			if(*s1 == '\0') return(1);
			s1++; s2++;
			continue;
		} else return(0);
	}
	return(0);
}
copy(s1, s2) char *s1, *s2; {
int i;

	for(i=0;i<5;++i) {
		*s2 = *s1;
		if(*s1 == '\0') return;
		s1++; s2++;
	}
	*s2 =  '\0';
}
ask() {
char c, ans;
	printf("Type `y' to override:     ");
	read(devtty, &c, 1);
	ans = c;
	while(c != '\n') read(devtty, &c,1);
	if(ans == 'y') return;
	else exit(9);
}

fscopy(fromnm, tonm) char *fromnm, *tonm; {
int i;
	block = 0;
	printf("From: %s, to: %s? (DEL if wrong)\n", fromnm, tonm);
	first = 1;
	sleep(10);  /* 10 seconds to DEL  */
	sync();
	from = open(fromnm,0);
	to = open(tonm,1);
	if(from<0 || to<0) {
		printf("open error: from(%d), to(%d)\n",from,to);
		exit(9);
	}
	for(; fs ; fs =- nblocks) {
		if(read(from,&buf, 512*nblocks)!=512*nblocks) {
			printf("Read error block %l...\n",block);
			for(i=0;i!=nblocks*256;++i) buf[i] = 0;
			if(!fromtape)
				seek(from, block+nblocks, 3);
		}
		if(first) {
			fs = buf[257];
			printf("filesystem size: %l\n",fs);
			first = 0;
			sptr = &buf[256];
			copy(supero.s_fpack,sptr->s_fpack);
		}
		if(write(to,&buf, 512*nblocks) != 512*nblocks) {
			printf("Write error block %l...\n",block);
			exit(9);
		}
		nblocks = fs > nblocks ? nblocks : fs;
		block =+ nblocks;
	}
	printf("END: %l blocks.\n", block);
}
gmatch(s, p)
char *s, *p;
{
	int c, cc, ok, lc, scc;

	scc = *s;
	lc = 077777;
	switch (c = *p) {

	case '[':
		ok = 0;
		while (cc = *++p) {
			switch (cc) {

			case ']':
				if (ok)
					return(gmatch(++s, ++p));
				else
					return(0);

			case '-':
				ok =| lc <= scc & scc <= (cc=p[1]);
			}
			if (scc==(lc=cc)) ok++;
		}
		return(0);

	case '?':
	caseq:
		if(scc) return(gmatch(++s, ++p));
		return(0);
	case '*':
		return(umatch(s, ++p));
	case 0:
		return(!scc);
	}
	if (c==scc) goto caseq;
	return(0);
}

umatch(s, p)
char *s, *p;
{
	if(*p==0) return(1);
	while(*s)
		if (gmatch(s++,p)) return(1);
	return(0);
}

