/*
 *	opr -- off line print via daemon 
 *
 *	modified by Kenj McDonell at University of Melbourne
 *	April 13, 1978.
 */

char	tfname[]	"/u/usr/lpd/tfaXXXXX";
char	cfname[]	"/u/usr/lpd/cfaXXXXX";
char	lfname[]	"/u/usr/lpd/lfaXXXXX";
char	dfname[]	"/u/usr/lpd/dfaXXXXX";
/*
 *	files used by the spooling subsystem
 *
 *	tfaXXXXX	temporary file of commands to the spooling daemon
 *	cf?XXXXX	a copy of a file to be spooled
 *	lf?XXXXX	a link to a file to be spooled
 *	dfaXXXXX	the actual commands file passed to the daemon
 *		"XXXXX" is the process id for the current
 *			invocation of opr
 *		"?" is 'a' for the first file argument, 'b' for the 
 *			second, 'c' for the third, etc.
 */
int	nact;		/*  number of files spooled for output  */
int	tff;		/*  file descriptor for "tfaXXXXX"  */
char	person[20];	/*  login user name  */
int	pid;		/*  process id  */
int	inchar;		/*  index to the character '?' in the file name strings  */
int	maxrec	1000;	/*  limit on the number of blocks to be copied across  */
int	debug;		/*  debug flag  */
int	now[2];		/*  the time the spool job is submitted  */
int	copies;		/*  number of copies  */
int	bcopies;	/*  number of banner pages per copy  */
int	copyflg;	/*  copy file into spool directory flag  */
int	remflg;		/*  remove file after spooling flag  */
char	banner[10];	/*  banner heading  */
char	mesg[31];	/*  all done message  */
char	daytime[30];	/*  date and time string  */
 
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
 
main(argc, argv)
int argc;
char *argv[];
{
	register char *arg;
	register char	*p;	/*  pointer to string following argument flag  */
	register char	*q;	/*  pointer for string copies  */
	register char	*end;	/*  end of string pointer for copies  */
	int out();
	int	f;		/*  temporary file descriptor  */
	struct ibuf	fstatb;		/*  buffer for "stat" call  */

	/*  build file names, with process id appended  */
	pidfn();
 
	/*  hangup, interrupt and quit signals are trapped by "out"  */
	if((signal(1, 1) & 01) == 0)
		signal(1, out);
	if((signal(2, 1) & 01) == 0)
		signal(2, out);
	if((signal(3, 1) & 01) == 0)
		signal(3, out);
 
	/*  create "tfaXXXXX"  */
	tff = nfile(tfname);
 
	/*  initialize control variables and flags  */
	copies = 1;
	bcopies = 1;
	loginid();
	q = banner;
	p = person;
	while (*q++ = *p++);
	mesg[0] = '\0';
 
	/*  scan arguments looking for flags  */
	while (argc>1 && (arg = argv[1])[0]=='-') {
		p = arg+2;
		switch (arg[1]) {

		case 'b':
			/*  user defined banner  */
			q = banner;
			end = q+9;
			while ((q < end) && (*q++ = *p++));
			banner[9] = '\0';
			break;
 
		case 'c':
			/*  copy file, rather than linking to it  */
			copyflg = 1;
			break;

		case 'd':
			/*  set debug flag  */
			debug = 1;
			break;
 
		case 'm':
			/*  write message after spooling  */
			q = mesg;
			end = q+30;
			while ((q < end) && (*q++ = *p++));
			mesg[30] = '\0';
			break;
 
		case 'n':
			/*  produce more than one copy  */
			if ((copies = atoi(p)) < 1) {
				printf ("Copies must be > 0\n");
				out();
			}
			break;
 
		case 'r':
			/*  remove the file (if possible) after spooling  */
			remflg = 1;
			break;

		case 'x':
			/*  define number of banner pages per copy  */
			bcopies = atoi(p);
			break;
 
		default:
			/*  invalid flag  */
			printf ("Bad option: %s\n",arg);
			out();
		}
		argc--;
		argv++;
	}
/*  debug  */
if (debug) {
	printf ("pid: %l\nperson: %s\nbanner: %s\n", pid, person, banner);
	printf ("mesg: %s\ncopies: %l\nremflg: %l\n", mesg, copies, remflg);
	printf ("copyflg: %l\n", copyflg);
	printf("bcopies: %l\nttyno: %c\n", bcopies, ttyn(0));
}
 
	/*
	 *  commands to the daemon
	 *
	 *	D		set debug flag
	 *	Lxxxx		login id = "xxxx"
	 *	Nnn		"nn" copies of each file
	 *	Bxxxx		banner heading = "xxxx"
	 *	Xxxxx		"xxxx" banner pages per copy
	 *	Mxxxx		send message "xxxx" when job done
	 *	Rxxxx		spooled file's real name is "xxxx"
	 *	Sxxxx		"xxxx" is the time/date the job was submitted
	 *	Tn		job submitted from tty no. "n"
	 *	Fxxxx		print file "xxxx"
	 *	Uxxxx		unlink file "xxxx"
	 *
	 *		output the global commands (D, L, T, N, B, X, M, T)
	 */
	if (debug) card('D',"");
	card('L', person);
	daytime[0] = ttyn(0);
	daytime[1] = '\0';
	card('T', daytime);
	if (copies > 1) {
		itoa(copies, daytime);
		card ('N', daytime);
	}
	if (bcopies != 1) {
		itoa(bcopies, daytime);
		card ('X', daytime);
	}
	if (banner[0] != '\0') card('B', banner);
	if (mesg[0] != '\0') card('M', mesg);
	time(now);
	p = daytime;
	arg = ctime(now);
	while ((*p++ = *arg++) != '\n');
	*--p = '\0';
	card ('S', daytime);
 
	if(argc == 1)
		/*  no input files specified as arguments .. use standard input  */
		copy(0);
	/*  pick up file names and process them one at a time  */
	while(--argc) {
		arg = *++argv;
 
		/*  skip empty files  */
		stat(arg, &fstatb);
		if (fstatb.isize == 0) goto df;
 
		/*  output real file name  */
		card('R', arg);
		if (copyflg)
			/*  user requested copy  */
			goto cf;
		if(link(arg, lfname) < 0)
			/*  could not link to file, so must copy it  */
			goto cf;
 
		/*  print via link, then unlink  */
		card('F', lfname);
		card('U', lfname);
		/*  ensure next link has a unique name  */
		lfname[inchar]++;
		nact++;
		goto df;
 
		/*  copy the file  */

	cf:
		f = open(arg, 0);
		if(f < 0) {
			printf("Cannot open %s\n", arg);
			continue;
		}
		copy(f);
		close(f);

		/*  if "-r" specified, remove the original file  */
	df:
		if (remflg) {
			f = unlink(arg);
			if(f < 0)
				printf("Cannot remove %s\n", arg);
		}
	}

	if(nact) {
		/*  
		 *  some files have been queued ....
		 *  rename tfaXXXXX to dfaXXXXX
		 *  then "fire up" the daemon
		 */
		tfname[inchar]--;
		f = link(tfname, dfname);
		if(f < 0) {
			printf("Cannot rename %s\n", dfname);
			tfname[inchar]++;
			out();
		}
		/*  inform user of spool job number  */
		printf ("Spool job no. %l\n", pid);
		/*  remove temporary daemon commands file  */
		unlink(tfname);
		/*  fire up the daemon  */
		execl("/etc/lpd", "lpd", 0);
		/*  woops  */
		printf("Cannot execute /etc/lpd\n");
		exit(1);
	}
	/*  exit here if no files were successfully queued  */
	out();
}

copy(f)
int f;
{
	/*  copy a file  */
	int ff, i, nr, nc;
	static char buf[512];

	card('F', cfname);
	card('U', cfname);
	ff = nfile(cfname);
	nc = 0;
	nr = 0;
	while((i = read(f, buf, 512)) > 0) {
		write(ff, buf, i);
		nc =+ i;
		if(nc >= 512) {
			/*  another block  */
			nc =- 512;
			nr++;
			if(nr > maxrec) {
				/*  too many blocks in the copy file  */
				printf("Copy file is too large .. truncated after %l bytes\n", nr*512+nc);
				break;
			}
		}
	}
	close(ff);
	nact++;
}

card(c, s)
int c;
char s[];
{
	char *p1, *p2;
	static char buf[512];
	int col;

	p1 = buf;
	p2 = s;
	col = 0;
	*p1++ = c;
	while((c = *p2++) != '\0') {
		*p1++ = c;
		col++;
	}
	*p1++ = '\n';
	write(tff, buf, col+2);
}

loginid()
{
	/*  extract the user's login id, and return it via 'person'  */
	register char *bp, *pp;
	static char buf[50];
 
	bp = buf;
	if(getpw((getuid() & 0377), bp)) {
		bp = "who?:";
	}
	pp = person;
	while((*pp++ = *bp++) != ':');
	*--pp = '\0';
}

pidfn()
{
	/*  build file names, with process id appended  */
	register i, j, c;
	int p;

	/*  get process id  */
	pid = getpid();
	p = pid;
	i = 0;
	while(tfname[i] != 'X')
		i++;
	i =+ 4;
	/*  insert process id as last 5 characters of the file names  */
	for(j=0; j<5; j++) {
		c = (p%10) + '0';
		p =/ 10;
		tfname[i] = c;
		cfname[i] = c;
		lfname[i] = c;
		dfname[i] = c;
		i--;
	}
	inchar = i;
}

nfile(name)
char *name;
{
	register f;
	/*  create a new file  */

	f = creat(name, 0666);
	if(f < 0) {
		printf("Cannot create %s\n", name);
		out();
	}
	/*
	 *  increment the 6th last character of the file name
	 *  -- this is required if multiple files are spooled in a single
	 *  execution of opr.
	 *  The resultant files names are of the form
	 *	.....aXXXXX, .....bXXXXX, .....cXXXXX, .....dXXXXX, etc
	 */
	name[inchar]++;
	return(f);
}
 
 
out()
{
	/*  
	 *  come here following :-
	 *	a) no files successfully queued
	 *	b) a fatal error
	 *	c) a hangup, interrupt, or quit signal.
	 *
	 *  remove all the files
	 */
	register i;

	signal(1, 1);
	signal(2, 1);
	signal(3, 1);
	i = inchar;
	while(tfname[i] != 'a') {
		tfname[i]--;
		unlink(tfname);
	}
	while(cfname[i] != 'a') {
		cfname[i]--;
		unlink(cfname);
	}
	while(lfname[i] != 'a') {
		lfname[i]--;
		unlink(lfname);
	}
	while(dfname[i] != 'a') {
		dfname[i]--;
		unlink(dfname);
	}
	exit();
}
 
itoa(ival, pbuf)
int ival;
char *pbuf;
{
	/*  convert an integer (ival) to a 7 character, null-terminated string (pbuf)  */
	char *p;
	int j, temp;
	temp = ival;
	p = pbuf+6;
	*p-- = '\0';
	for (j=1; j<7; j++) {
		*p-- = (temp%10) + '0';
		temp =/ 10;
	}
	return;
}
