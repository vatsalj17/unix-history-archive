/*
 *	opq  --  interrogation of the spool queue
 *
 *	Written by Kenj McDonell, April 17 1978.
 *
 */
char lpd[]	"/u/usr/lpd";		/*  spool directory  */
char jobf[]	"/u/usr/lpd/job";	/*  active job number file  */
char dbuf[16];				/*  directory buffer  */
int cmdbuf[131];			/*  spool command buffer  */
char lbuf[64];				/*  for spool command lines  */
char person[20];			/*  login id of spool job submitter  */
char submitted[30];			/*  date/time spool job submitted  */
char realfname[64];			/*  real spool job file name  */
 
struct ibuf {
	int	idev;
	int	inum;
	int	iflags;
	char	inl;
	char	iuid;
	char	igid;
	char	isize0;
	int	isize;
	int	iaddr[8];
	char	*iatime[2];
	char	*imtime[2];
	int	filler;		/***/
};
 
int active;		/*  active job number  */
int job;		/*  report job number  */
int copies;		/*  number of copies per spool file  */
int lflag;		/*  long report format  */
int sflag;		/*  spool file names  */
int aflag;		/*  all jobs  */
 
main(argc, argv)
int argc;
char *argv[];
{
	char *arg;
	int f;		/*  file descriptor  */
	int found;	/*  no. jobs located  */
	int i;
	int fd;		/*  spool directory file descriptor  */
	int format;	/* report format flag  */
 
	/*  open the directory  */
	if ((fd = open(lpd, 0)) == -1) {
		/*  woops  */
		printf ("Cannot open %s\n", lpd);
		exit(1);
	}
 
	/*  move to the spool directory  */
	if (chdir(lpd) < 0) exit(1);
 
	/*  locate the active job, if any  */
	if ((f = open(jobf, 0)) != -1) {
		read(f, dbuf, 6);
		close(f);
		active = atoi(dbuf);
	}
 
	/*  set up report flags  */
	while (argc > 1 && (arg = argv[1])[0] == '-') {
		for (i = 1; arg[i] != '\0'; i++)
		switch (arg[i]) {
 
		/*  long format  */
		case 'l':
			lflag = 1;
			break;
 
		/*  list all jobs  */
		case 'a':
			aflag = 1;
			break;
 
		/*  list spool file names  */
		case 's':
			sflag = 1;
			break;
 
		/*  bad option  */
		default:
			printf("Bad option: %s\n", argv[1]);
			exit(1);
 
		}
		argc--;
		argv++;
 
	}
 
	/*  examine each entry in turn, looking for spool command files  */
	while (read(fd, dbuf, 16) == 16) 
	    if (dbuf[0] | dbuf[1] != 0 && dbuf[2] == 'd' && dbuf[3] == 'f') {
		/*  found a command file  */
		found++;
		job = atoi(&dbuf[5]);
		if (argc == 1) {
			/*  no job numbers specified  */
			report();
			continue;
		}
 
		for (i = 1; i < argc; i++) {
			/*  test each argument for equality  */
			if (job == atoi(argv[i])) {
				report();
				continue;
			}
		}
	}
 
	/*  check for line printer not all there ...  */
	if (active == 0 && found > 0) printf("Line printer off-line?\n");
}
 
report()
{
	int fd;
	char c;
	int flag;
	struct ibuf statb;
 
	if (fopen(&dbuf[2], cmdbuf) < 0) {
		/*  woops .. cannot access spool commands file  */
		printf("Cannot open %s\n", &dbuf[2]);
		return;
	}
 
	/*  check owner of commands file  */
	if (!aflag) {
		stat(&dbuf[2], &statb);
		if (statb.iuid != getuid()) return;
	}
 
		flag = 1;
		copies = 1;
		submitted[0] = '\0';
		realfname[0] = '\0';
		person[0] = '\0';
	/*  loop round here extracting  commands  */
	for (;;) {
		switch (c = getc(cmdbuf)) {
 
		/*  print a file  */
		case 'F':
			getline(lbuf);
			if (flag) {
				flag = 0;
				printf("%6d", job);
				if (job == active) printf("*");
				else printf(" ");
				if (lflag) {
					printf(" %s", person);
					submitted[16] = '\0';
					printf("\t%s", &submitted[4]);
				}
			}
			else if (lflag || sflag) {
				printf("       ");	/*  job number  */
				if (lflag) printf("   \t            ");		/*  user and date/time  */
			}
			if (lflag) {
				if (stat(lbuf, &statb) < 0) statb.isize = 0;
				printf(" %9d", statb.isize);
				printf(" %2d", copies);
			}
			if (sflag) printf("  (%s)", &lbuf[11]);
			if (lflag) printf(" %s", realfname);
			printf("\n");
			if (lflag || sflag) continue;
			close(cmdbuf[0]);
			return;
 
		/*  login person id  */
		case 'L':
			getline(person);
			continue;
 
		/*  number of copies  */
		case 'N':
			getline(lbuf);
			copies = atoi(lbuf);
			continue;
 
		/*  real file name  */
		case 'R':
			getline(realfname);
			continue;
 
		/*  time submitted  */
		case 'S':
			getline(submitted);
			continue;
 
		/*  end of command file  */
		case -1:
			close(cmdbuf[0]);
			return;
 
		/*  ignore all other commands  */
		default:
			getline(lbuf);
			continue;
		}
	}
 
}

/*
 * Read rest of line from command file
 */
getline(buf)
char buf[];
{
	register char *p;
	register c;

	for (p = buf; (*p = c = getc(cmdbuf)) != '\n' && c > 0; p++)
		;
	*p = '\0';
}
