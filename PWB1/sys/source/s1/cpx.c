#
/*
 * Copy a file
 */
#include <stdio.h>
#include <stat.h>
#define MAGIC 076907
	long	itol();
	long	nbrchar, sumchar;
	struct statb statb,statb1;
	struct {
		int	magic;	/* always MAGIC */
		char	name[129];
		int	hflags;
		char	huid, hgid;
		char	null;
		long	hmtime;
		long	nbrchar;
		long	sumchar;
	} hdr;
main(argc, argv) char **argv; {
	int	f1, f2, i, ansflg;
	FILE *f1id, *f2id;
	char	c;
	f1=f2=ansflg=0;
	if(argc != 3) {
		fprintf(stderr,"Usage: cpx {file1 | -} {file2 | - | .}\n");
		exit(1);
	}
	if(argv[1][0] == '-') {
		f1++;
	}
	if(argv[1][0] == '+') {
		ansflg++; f1++;
	}
	if(argv[2][0] == '-') {
		f2++;
	}
	if(f1 == 0 && f2 == 0) {
		if(stat(argv[1],&statb) != 0) {
			fprintf(stderr,"stat error %s\n",argv[2]);
			exit(1);
		}
		if(stat(argv[2],&statb1) == 0) {
			if(statb.i_dev == statb1.i_dev && statb.i_ino == statb1.i_ino) {
				fprintf(stderr,"Copying file to itself\n");
				exit(1);
			}
		}

	}
/*
 * Setup input file
 */
	f1id = stdin;
	if(f1 == 0) {
		if((f1id=file(argv[1]))==1) {
			fprintf(stderr,"Can't copy directory.\n");
			exit(1);
		}
		if(f1id == -1) {
			fprintf(stderr,"File not found: %s\n", argv[1]);
			exit(1);
		}
		hdr.magic = MAGIC;
		strcpy(hdr.name, argv[1]);
		hdr.hflags = statb.i_mode;
		hdr.huid = statb.i_uid;
		hdr.hgid = statb.i_gid;
		hdr.hmtime = statb.i_mtime;
		fclose(stdin);
		if((f1id = fopen(argv[1],"r"))==NULL) {
			fprintf(stderr,"Can't open %s\n", argv[1]);
			exit(1);
		}
	}
	else {
		fread(&hdr, sizeof hdr, 1, f1id);
		if(ferror(f1id)) {
			fprintf("READ ERROR\n");
			exit(1);
		}
		if(hdr.magic != MAGIC) {
			fprintf(stderr,"Illegl input\n");
			exit(1);
		}
	}
/*
 * Setup output file
 */
	f2id = stdout;
	if(f2 == 0) {
		i = 0;
		if((f2id=file(argv[2]))==1) {
			if(chdir(argv[2])<0) {
				fprintf(stderr,"Can't use %s\n", argv[2]);
				exit(1);
			}
			else {
				i = findname(hdr.name);
				strcpy(hdr.name, &hdr.name[i]);
				if((f2id=file(hdr.name))==1) {
					fprintf(stderr,"%s/%s is a directory.\n",
						argv[2], hdr.name);
					exit(1);
				}
			}
		}
		else {
			strcpy(hdr.name, argv[2]);
		}
		fclose(stdout);
		if((f2id = fopen(hdr.name,"w")) == NULL) {
			fprintf(stderr,"Can't create new file %s\n", hdr.name);
			exit(1);
		}
}
/*
 * case 1:	cpx file1 file2
 */
	if(f1 == 0 && f2 == 0) {
		copyfile(f1id, f2id);
		chghdr();
		exit(0);
	}
/*
 * case 2:	cpx - file2
 */
	if(f1 == 1 && f2 == 0) {
		copyfile(f1id, f2id);
		chghdr();
		if(nbrchar != hdr.nbrchar || sumchar != hdr.sumchar) {
			fprintf(stderr,"Check output for error.\n");
			exit(1);
		}
		if(ansflg) {
			fprintf(stderr,"ok\n");
		}
		exit(0);
	}
/*
 * case 3*	cpx file1 -
 */
	if(f1 == 0 && f2 == 1) {
		while((c=getc(f1id)) != EOF) {
			if(ferror(f1id)) {
				fprintf(stderr,"Read error\n");
				exit(1);
			}
			hdr.nbrchar =+ 1;
			hdr.sumchar =+ c;
		}
		rewind(f1id);
		fwrite(&hdr, sizeof hdr, 1, f2id);
		if(ferror(f2id)) {
			fprintf("WRITE ERROR\n");
			exit(1);
		}
		copyfile(f1id, f2id);
		exit(0);
	}
/*
 * case 4:	cpx - -
 */
	exit(1);
}
/*
 * find file and determine type:
 *	-1 = file not found, 0 = ordinary file, 1 = directory.
 */
file(fname) {
	if(stat(fname, &statb) != 0)
		return(-1);
	if((statb.i_mode&IFMT)==IFDIR)
		return(1);
	return(0);
}
/*
 * Find lowest level file name in path
 */
findname(s)
char *s; {
	int n, i;
	for(i=n=0; *s != '\0'; n++) {
		if(*s++ == '/')
			i=n+1;
	}
	return(i);
}
/*
 * change header
 */
chghdr() {
	chown(hdr.name, hdr.huid);
	chmod(hdr.name, hdr.hflags);
}
/*
 * copy file
 */
copyfile(f1id, f2id) {
	char c;
	while((c=getc(f1id)) != EOF) {
		if(ferror(f1id)) {
			fprintf("Read error\n");
			exit(1);
		}
		nbrchar =+ 1;
		sumchar =+ c;
		putc(c, f2id);
		if(ferror(f2id)) {
			fprintf("Write error\n");
			exit(1);
		}
	}
}
