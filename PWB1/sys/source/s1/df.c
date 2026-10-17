#
#define NMOUNT 20
#define MTABSIZE 26

struct mtab {
	char	fsys[10];
	char	node[10];
	int	ro_flg;
	int	date[2];
	} mtab[NMOUNT];

struct	filsys
{
	char	*s_isize;
	char	*s_fsize;
	int	s_nfree;
	int	s_free[100];
	int	s_ninode;
	int	s_inode[100];
	char	s_flock;
	char	s_ilock;
	char	s_fmod;
	char	s_ronly;
	int	s_time[2];
	int	pad[40];
	int	s_tfree;
	int	s_tinode;
	char	s_fname[6];
	char	s_fpack[6];
};
struct filsys sblock;
int	fi;
int	uflag, tflag, nfree, num, status, qflag, sflag;
int	fsize, isize, tinode;
char	*ch;
int	statb[18];

main(argc, argv)
char **argv;
{
	register struct mtab *mp;
	char *nptr, *pfsys, *tnptr, *pnode;
	int i, rec, k, ndtflg, nameflg;

if( argc > 1 && *argv[1] == '-') { /* options arg */
	ch = argv[1];
	while(*++ch != '\0') {
		switch(*ch) {

		case 'u':	/* File System Usage Info. */
			uflag++;
			continue;
		case 't':	/* Comparison */
			tflag++;
			continue;
		case 'q':	/* Quick Free Space Scan */
			qflag++;
			continue;
		case 's':	/* Inhibit print */
			sflag++;
			continue;
		default:
			printf("%c not implemented\n",*ch);
			break;
		}
	}
	argc--;
	argv++;
	}
	status = 0;
	if(tflag) {
		comp(*++argv);
		if(num < 0) {
			printf("ERROR - illegal argument\n");
			exit(1);
		}
	nfree = num;
	argc--;
	}
	if((rec=open("/etc/mnttab",0)) <0) {
		write(1,"cannot open\n",12);
		exit(1);
	}
	if((read(rec,&mtab[0],sizeof mtab)) < 0) {
		write(1,"bad file\n",9);
		exit(1);
	}
	if(argc <= 1) {
		chdir("/dev");
		for(mp=mtab;mp<&mtab[NMOUNT];mp++) {
			if(mp->node[0] != 0) {
				if(!uflag && !tflag && !qflag) {
					if(!sflag)printf("/dev/%s (%s) ",
						mp->fsys,mp->node);
					dfree(mp->fsys," ");
				}
				else if(!tflag) dfree(mp->fsys,mp->node);
				if(tflag) dfree(mp->fsys," ");
			}
		}
	}
	if(argc >1) {
		for(k=1;k<argc;k++) {
			chdir("/dev");
			ndtflg = 0; nameflg = 0;
			nptr = *++argv;
			stat(*argv,statb);
			if(((statb[2]&020000)==020000) ||
				((statb[2]&060000)==060000))nameflg = 0;
			else nameflg++;
			while(*nptr++ != '\0');
			nptr--;
			while(*--nptr == '/')
				*nptr = '\0';
			while(nptr > *argv && *--nptr != '/');
			if(!nameflg) {
				if(*nptr == '/') nptr++;
			}
			for(mp=mtab;mp<&mtab[NMOUNT];mp++) {
				if(mp->node[0] != 0) {
					if(!nameflg) {
						pfsys = mp->fsys;
						tnptr = nptr;
						while(*pfsys++ == *tnptr++) {
							if(*pfsys == '\0' &&
								*tnptr == '\0') {
								ndtflg++;
								break;
							}else continue;
						}
						if(ndtflg) break;
						else continue;
					}
					if(nameflg) {
						pnode = mp->node;
						if(*nptr == '\0')*nptr = '/';
						tnptr = nptr;
						while(*pnode++ == *tnptr++){
							if(*pnode == '\0' &&
							   *tnptr == '\0') {
								ndtflg++;
								break;
							}
						}
						if(ndtflg) break;
						continue;
					}
				}
			}
			if(ndtflg && !nameflg) {
				if(!uflag && !tflag && !qflag) {
					if(!sflag)printf("/dev/%s (%s) ",
						mp->fsys,mp->node);
					dfree(mp->fsys," ");
				}
				else dfree(mp->fsys,mp->node);
			}
			if(!ndtflg && !nameflg) {
				if(!uflag && !tflag && !qflag) {
					if(!sflag)printf("/dev/%s (%s) ",
							nptr," ");
				}
				 dfree(nptr," ");
			}
			if(nameflg && ndtflg) {
				if(!uflag && !tflag && !qflag){
					if(!sflag)printf("/dev/%s (%s) ",
						mp->fsys,mp->node);
					dfree(mp->fsys," ");
				}
				else dfree(mp->fsys,mp->node);
			}
		}
	}
	exit(status);
}

dfree(file,name)
char *file, *name;
{
	int i;
	char fmaj, fmin, tbuf[36];

	fi = open(file, 0);
	if(fi < 0) {
		printf("cannot open %s\n", file);
		return;
	}
	sync();
	bread(1, &sblock);
	if(qflag && !tflag && !uflag) {
		if(!sflag)printf("/dev/%s (%s) %l\n",file,name,sblock.s_tfree);
		close(fi);
		return;
	}
	if(uflag) {fsize = sblock.s_fsize; isize = sblock.s_isize+2;
					   tinode = sblock.s_tinode;}
	if(!qflag) {
		i = 0;
		while(alloc()) {
			i++;
			if(tflag && i > nfree)
				break;
		}
	}else i = sblock.s_tfree;
	if(tflag) {
		if(stat(file,tbuf) == -1) {
			printf("/dev/%s stat error\n",file);
		}
		fmaj = tbuf[13]; fmin = tbuf[12];
		if(i > nfree) {
			if(!sflag)printf("%2d %2d Y\n",fmaj,fmin);
		}
		else {
			if(!sflag)printf("%2d %2d N\n",fmaj,fmin);
			status = 1;
		}
		close(fi);
		return;
	}
	if(uflag && !sflag) {
		printf("/dev/%s(%s)\t%5l total blocks\n",file,name,fsize);
		printf("\t\t%5l system use\n",isize);
		printf("\t\t%5l free\n",i);
		printf("\t\t%5l used\n",fsize-isize-i);
		printf("\t\t%5l free inodes\n\n",tinode);
		}
		else if(!sflag)printf("%l\n",i);
	close(fi);
}

alloc()
{
	char	*b;
	int	i;
	int buf[256];

	i = --sblock.s_nfree;
	if(i<0 || i>=100) {
		if(!tflag) printf("bad free count\n");
		return(0);
	}
	b = sblock.s_free[i];
	if(b == 0)
		return(0);
	if(b<sblock.s_isize+2 || b>=sblock.s_fsize) {
		if(!tflag) printf("bad free block (%l)\n", b);
		return(0);
	}
	if(sblock.s_nfree <= 0) {
		bread(b, buf);
		sblock.s_nfree = buf[0];
		for(i=0; i<100; i++)
			sblock.s_free[i] = buf[i+1];
	}
	return(b);
}

bread(bno, buf)
{
	int n;
	extern errno;

	seek(fi, bno, 3);
	if((n=read(fi, buf, 512)) != 512) {
		if(!tflag) {
			printf("read error %d\n", bno);
			printf("count = %d; errno = %d\n", n, errno);
		}
		exit(1);
	}
}
comp(ap)
char *ap;
{
	register char *p;
	int compflag, count; 
	p = ap; num = 0; compflag = 0; count = 1;
	while(*p != '\0' && count <= 5) {
		if(*p >= '0' && *p <= '9') {
			num = num*10 + *p++ - '0';
			count++;
		}
		else {
			compflag++;
			break;
		}
	}
	if(compflag) num = -1;
	return(num);
}
