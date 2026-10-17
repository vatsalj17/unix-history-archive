struct {
	char cname[8];
	char lname[6];
	char shtty;
	char tuid;
	int datet[2];
	int realt[2];
	int bcput[2];
	int bsyst[2];
} tbuf;

char afname[] "/etc/sha\0xxxxxxxxxxxxxxxxxxxxxxxxxxxx";
int pn;
int f;

int ttyflg, nameflg, allflg, recscan 1000;
char *ap;
char ctty[2];
int *ta;

main(argc, argv) char **argv; {

	register i, ct;

	if(argc<2) {
	BAD:
		write(2, "Usage: lastcom [+] [++] [ttyletter | -logname]\n", 47);
		exit(1);
	}

    while(argc>1) {
	if(strcmp(argv[1], "+")==0) {
		allflg++;
		argv++;
		argc--;
		continue;
	} else if(strcmp(argv[1], "++")==0) {
		recscan = 32000;
		argv++;
		argc--;
		continue;
	} else if(strcmp(argv[1], "-f")==0) {
		strcpy(afname, argv[2]);
		argv++;argv++; argc=- 2;
		continue;
	}
	if(argv[1][0]=='-') {
		ap = &argv[1][1];
		nameflg++;
		break;
	} else if(argc==2) {
		ttyflg++;
		ap = argv[1];
		break;
	} else if(!allflg)
		goto BAD;

  }
	f = open(afname, 0);
	seek(f, 0, 5);
	printf("TTY  NAME   COMMAND   HR:MN:SC  (TICS)\n");

	for(pn = sizeof tbuf * 2; seek(f, -pn, 1)!=-1;) {
		read(f, &tbuf, sizeof tbuf);
		ctty[0] = tbuf.shtty;
		tbuf.shtty = '\0';
		if(allflg && !(ttyflg || nameflg))
			eprint();
		else if(ttyflg) {
			if(!*ap) exit(0);
			for(i=0;ap[i];++i)
				if(ap[i]==ctty[0]) {
					eprint();
					if(allflg) break;
					for(;ap[i];++i)
						ap[i]=ap[i+1];
					break;
				}
		} else if(nameflg && gmatch(tbuf.lname, ap))
			eprint();
		if(ct++ > recscan) break;
	}
	if(allflg) exit(0);
	printf("No <%s> after %s", argv[1], ctime(tbuf.datet));
	exit(1);
}

eprint() {
	printf(" %s  ", ctty);
	printf(" %-6s ", tbuf.lname);
	tbuf.lname[0] = 0;
	printf("%-8s  ", tbuf.cname);
	ta = localtime(tbuf.datet);
	printf("%2d:%2d:%2d  (%l)\n",ta[2],ta[1],ta[0],
		tbuf.bcput[1]+tbuf.bsyst[1]);
	if(allflg) return;
	if(ttyflg && *ap) return;
	exit(0);
}
