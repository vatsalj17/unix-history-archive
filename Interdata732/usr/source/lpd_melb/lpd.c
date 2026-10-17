/*
 *		Line printer daemon
 *
 *	Modified by Kenj McDonell, 14 April 1978
 *
 */

/* File names */
char lpd[]	"/u/usr/lpd";		/*  spooler directory  */
char lock[]	"/u/usr/lpd/lock";	/*  existence of this file prevents multiple, concurrent daemons  */
char printer[]	"/dev/pr";
char kill[]	"/u/usr/lpd/kill";	/*  kill/restart/pause/continue current job flag  */
char jobf[]	"/u/usr/lpd/job";	/*  current spool job no. written here  */
char tty[]	"/dev/tty?";		/*  spool job owner's terminal  */
char amesg[]	"Spool job no. XXXXX aborted";
char rmesg[]	"Spool job no. XXXXX restarted";

/* Buffers */
int	cmdbuf[131];		/* for reading command file */
char	prbuf[512];		/* for copying file to printer */
char	lbuf[64];		/* for assembling lines from command file */
char	dbuf[17];		/* for reading directory */
char	killbuf[8];		/*  commands from opctl  */
char	person[20];		/*  login user name  */
char	now[2];			/*  used to fetch current time  */
char	daytime[30];		/*  date and time string  */
char	banner[10];		/*  banner heading string  */
char	mesg[31];		/*  end of job message to user  */
char	submitted[30];		/*  date and time submitted  */
char	realfname[64];		/*  real spooled file name  */
char	ttyno;			/*  tty no. spool job submitted from  */
 
int debug;			/*  debug flag  */

main()
{
	register flag;
	extern int fout;
	int fd;

	/* Ignore quit, interrupt, hangup */
	signal(1, 1);
	signal(2, 1);
	signal(3, 1);

	/* Use lock to prevent multiple active daemons */
	if (creat(lock, 0) < 0)
		exit(0);

	/* Fork and exit, thus spawning the daemon and letting opr terminate  */
	if (fork())
		exit(0);

	/* Before opening the printer, make sure there's something to print */
	close(0);
	close(1);
	close(2);
	/*
	 *	assumes following file descriptor assignments will occur OK
	 *	0  -  spool directory
	 *	1  -  printer
	 */
	if (open(lpd, 0) != 0)
		dexit();
	flag = 0;
	while (read(0, dbuf, 16) == 16)
		if ((dbuf[0] | dbuf[1]) != 0
		    && dbuf[2] == 'd' && dbuf[3] == 'f')
			flag++;
	if (!flag)
		dexit();

	/*  Open the printer  */
	if (open(printer, 1) != 1)
		dexit();
	dup(1);
	fout = dup(1);

	/* Search lpd directory for work to do */
	if (chdir(lpd) < 0)
		dexit();
	seek(0, 0, 0);
	while (read(0, dbuf, 16) == 16) {
		if ((dbuf[0] | dbuf[1]) == 0
		    || dbuf[2] != 'd' || dbuf[3] != 'f')
			continue;

		/*
		 *  Process one command file.
		 *  First write the spool job number into "jobf".
		 */
		if ((fd = creat(jobf, 0666)) != -1) {
			write (fd, &dbuf[5], 5);
			write (fd, "\n", 1);
		}
		spool(&dbuf[2]);
 
		/*
		 *  All done, now remove the job no. and command files
		 */
		close (fd);
		unlink(jobf);
		unlink(&dbuf[2]);

		/* Reread directory in case it changed while we were spooling */
		seek(0, 0, 0);
	}

	dexit();
}

/*
 * Remove job no. and lock files,  and terminate
 */
dexit()
{
	unlink(jobf);
	unlink(lock);
	exit(0);
}

/*
 * Read and obey a file of spooling commands
 */
spool(dfname)
char *dfname;
{
	register in, len, c;
	int job, copies, copyno;
	int bcopies, j, n;
	char *p, *q;
 
	/*
	 *	All the work is really done in here
	 */
 
	if (fopen(dfname, cmdbuf) < 0)
		return;

	/*
	 *	Now the command file has been successfully opened,
	 *	initialize all relvant strings and flags, then
	 *	process the commands serially.
	 */
 
	job = atoi(&dbuf[5]);
	copies = 1;
	bcopies = 1;
	banner[0] = '\0';
	mesg[0] = '\0';
	person[0] = '\0';
	submitted[0] = '\0';
	realfname[0] = '\0';
	ttyno = '\0';
	debug = 0;
 
	/*  fetch and process commands  */
	for(;;) {
		switch (c = getc(cmdbuf)) {
	
		/*  banner  */
		case 'B':
			getline(banner);
			continue;
 
		/*  set debug flag  */
		case 'D':
			debug = 1;
			continue;
 
		/*  output a file  */
		case 'F':
			getline(lbuf);
			if ((in = open(lbuf, 0)) < 0) continue;
			if (debug) {
				printf ("job: %l\n", job);
				printf ("person: %s\n", person);
				printf("ttyno: %c\n", ttyno);
				printf ("real file: %s\n", realfname);
				printf ("banner: %s\n", banner);
				printf ("banner copies: %l\n", bcopies);
				printf ("submitted: %s\n", submitted);
				printf ("released: %s\n", daytime);
				printf ("copies: %l\n", copies);
			}
			for (copyno = 1; copyno <= copies; copyno++) {
				goto newcopy;
			restart:
				/*  current job has been restarted  */
				p = &dbuf[5];
				q = &rmesg[14];
				while (*q++ = *p++);
				*--q = ' ';
				sendmesg(person, ttyno, rmesg);
				printf("\n\n* * * Spool Job Restarted * * *\n");
			newcopy:
				time(now);
				p = daytime;
				q = ctime(now);
				while ((*p++ = *q++) != '\n');
				*--p = '\0';
				for (j = 0; j < bcopies; j++) {
					bannerpage(job, copyno, copies);
					if ((n = check(job)) != 0) {
						/*  opctl has given us the word  */
						if (n == 1) goto restart;
						goto killed;
					}
				}
 
				/*  flush the buffer before copying  */
				flush();
 
				/*  the output is done here  */
				seek (in, 0, 0);
				while ((len = read(in, prbuf, 512)) > 0
				    && write(1, prbuf, len) == len) 
					if ((n = check(job)) != 0) {
						/*  opctl has decreed that we restart or abort  */
						if (n == 1) goto restart;
						/*  woops .. abandon the current output file  */
			killed:
						p = &dbuf[5];
						q = &amesg[14];
						while (*q++ = *p++);
						*--q = ' ';
						sendmesg(person, ttyno, amesg);
						printf("\n\n* * * Spool Job Aborted * * *\n");
						copyno = copies;
						break;
					}
				/*  eject the last page  */
				putchar('\014');
			}
			close(in);
			continue;
	
		/*  login user id  */
		case 'L':
			getline(person);
			continue;
 
		/*  end of job message  */
		case 'M':
			getline(mesg);
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
 
		/*  tty no.  */
		case 'T':
			getline(lbuf);
			ttyno = lbuf[0];
			continue;
 
		/*  number of banner pages per copy  */
		case 'X':
			getline(lbuf);
			bcopies = atoi(lbuf);
			continue;
 
		/* Delete file after copying */
		case 'U':
			getline(lbuf);
			unlink(lbuf);
			continue;
 
		/*  end of command file  */
		case -1:
			close(cmdbuf[0]);
			flush();
			sendmesg(person, ttyno, mesg);
			return;
 
		/*  unidentified command  */
		default:
			getline(lbuf);
			/*  just ignore it  */
			continue;
		}
	}
	close(cmdbuf[0]);
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
 
bannerpage(job, copyno, copies)
int job, copyno, copies;
{
	int j;
	char c;
	register char *p, *q;
	/*  
	 *	Output the banner page headings.
	 */
	putchar('\014');
	p = realfname;
	q = realfname;
	while (c = *q++) if (c == '/') p = q;
	/*  'p' points past the last '/'  */
	q = q-1;
	/*  'q' points at the end of the string  */
	if (q > p+9) {
		/*  truncate from right  */
		q = p+9;
		*q = '\0';
	} else
	if (q < p+9) {
		/*  extend to the left  */
		p = q-9;
		if (p < realfname) p = realfname;
	}
 
	q = banner;
 
	if (*p != '\0') bprint(p);
	else if (*q != '0') bprint(q);
	else printf (".\n\n\n\n\n\n\n\n\n\n\n\n\n");
	chars(' ');
	chars('!');
	chars('"');
	printf ("\n\n");
	printf ("Computer Science Department, Melbourne University\n\n");
	printf ("\t\tSpool Job Number:  %l\n", job);
	printf ("\t\tSubmitted:  %s\n\t\tReleased:   %s\n", submitted, daytime);
	printf ("\t\tCopy  %l  of  %l\n\n\n", copyno, copies);
	chars('#');
	chars('$');
	chars('%');
	printf("\n\n");
	/*  print the banner if it did not appear at the top of the page  */
	if (*p != '\0' && *q != '\0') bprint(q);
	putchar('\014');
	return;
}
check(job)
int job;
{
	/*
	 *	Check if opctl has sent us a message
	 */
	int fd, paused, killjob;
	paused = 0;
again:
	if ((fd = open(kill, 0)) != -1) {
		/*  "kill" file exists  */
		read(fd, killbuf, 6);
		killjob = atoi(killbuf);
		/*  get rid of the file  */
		close(fd);
		unlink(kill);
		if (killjob == job) {
			/*  this is really meant for me!  */
			/*  the following control functions are supported :-
			 *	'p'	pause the cUrrent job
			 *	'a'	abort the current job
			 *	'c'	continue the current job
			 *	'r'	restart the current job
			 */
			switch (killbuf[5]) {
 
			/*  pause  */
			case 'p':
				paused = 1;
				break;
 
			/*  abort  */
			case 'a':
				/*  abort this print file  */
				return(-1);
 
			/*  continue  */
			case 'c':
				return(0);
 
			/*  restart  */
			case 'r':
				return(1);
		
			}
 
		}
		/*  note: if "kill" does not refer to the current spool job, ignore it  */
	}
	/*  is paused, wait for 5 seconds and try again, else return  */
	if (paused) {
		sleep(5);
		paused++;
		if (paused < 24) goto again;
		/*  waited too long ... abort him  */
		return(-1);
	}
	return(0);
}
 
chars(c)
char c;
{
	/*
	 *	Print a line of 128 characters, starting with 'c'
	 */
	int j;
	char ch;
	ch = c;
	for (j = 0; j < 128; j++) {
		putchar(ch++);
		if (ch > '~') ch = ' ';
	}
	putchar('\n');
}
 
sendmesg(loginid, ttyno, message)
char loginid[];
char ttyno;
char message[];
{
	/*	
	 *	Send a message to a user
	 */
	int fd, len;
	register char *p, *q;
	if (ttyno == 'x') return;
	if (message[0] != '\0' && (fd = open("/etc/utmp", 0)) != -1) {
		/*  something to do  */
		while ((len = read(fd, lbuf, 24)) == 24)
			if (lbuf[0] != '\0') {
				p = loginid;
				q = lbuf;
				while (*p++ == *q++);
				if (*--p == '\0' && lbuf[8] == ttyno) {
					tty[8] = lbuf[8];
					close(fd);
					if ((fd = open(tty,1)) != -1) {
						p = message;
						while (*p != '\0') write (fd, p++, 1);
						write (fd, "\n", 1);
						close(fd);
					}
					return;
				}
			}
		close(fd);
	}
	return;
}
