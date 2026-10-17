/*
 *	opctl  --  asynchronous control of the line printer daemon
 *		   and the spooled output queue.
 *
 *	Written by Kenj McDonell, 20 April 1978.
 *
 */
char kill[]	"/u/usr/lpd/kill";	/*  used for cammands to the daemon  */
char jobf[]	"/u/usr/lpd/job";	/*  the current spool job number  */
char lpd[]	"/u/usr/lpd";		/*  spool directory  */
char dbuf[17];				/*  directory buffer  */
char job[6];				/*  job number of spool job to control  */
char active[6];				/*  active spool job number  */
 
int jobn;				/* integer equivalent of "job"  */
char flag;	/*   control action flag  */
 
main(argc, argv)
int argc;
char *argv[];
{
 
	if (argc < 2 || *argv[1] != '-') {
		/*  woops, syntax incorrect  */
		printf ("Usage: opctl [-arpc] [job no.]\n");
		exit(1);
	}
	flag = *(argv[1]+1);
	switch (flag) {
 
	/*  default => error  */
	default:
		printf("Bad option: %s\n", argv[1]);
		exit(1);
 
	/*  all the spool job control functions  */
	case 'a':
	case 'r':
	case 'p':
	case 'c':
		break;
 
	/*  kick the daemon  */
	case 'k':
		execl("/etc/lpd", "lpd", 0);
		printf("Cannot execute /etc/lpd\n");
		exit(1);
	}
 
	if (argc == 2) {
		/*  control the active spool job  */
		ctl("*");
	}
	else 
		/*  control the cited jobs  */
		while (argc > 2) {
			ctl(argv[2]);
			argc--;
			argv++;
		}
}
 
ctl(job)
char job[];
{
	/*
	 *	Do the control functions.
	 */
	int fd;
	int found;
	/*  look for the currently active job  */
	if ((fd = open(jobf, 0)) < 0) {
		/*  no active job  */
		active[0] = '\0';
		if (job[0] == '*' || flag != 'a') {
			/*  tough luck!  */
			printf("No active spool job\n");
			exit(1);
		}
	}
	else {
		/*  extract the job number of the active job  */
		read (fd, active, 5);
		close(fd);
		active[5] = '\0';
	}
 
	if (job[0] == '*' || atoi(job) == atoi(active)) {
		/*  our lad is the active spool job  */
		if (open(kill, 0) > -1) {
			/*  "kill" already exists - cannot continue  */
			printf("Previous control function pending\n");
		return;
		}
		else {
			/*  "kill" not there - all's well  */
			if ((fd = creat(kill, 0666)) == -1) {
				/*  woops  */
				printf("Cannot open %s\n", kill);
				exit(1);
			}
 
			/*  send the command to the daemon  */
			write(fd, active, 5);
			write(fd, &flag, 1);
			write (fd, "\n", 1);
			close(fd);
			return;
		}
	}
 
	/*  it is not the active job  */
	if (flag != 'a') {
		/*  can only abort queued jobs  */
		printf("Job no. %s not active\n", job);
		return;
	}
 
	/*  delete all the files  */
	jobn = atoi(job);
	itoa(jobn, active);
	found = 0;
	/*  scan all spool directory entries  */
	if ((fd = open(lpd, 0)) == -1) exit(1);
	if (chdir(lpd) == -1) exit(1);
	while (read(fd, dbuf, 16) == 16)
		if (dbuf[0] | dbuf[1] != 0 &&
		    dbuf[5] == active[0] &&
		    dbuf[6] == active[1] &&
		    dbuf[7] == active[2] &&
		    dbuf[8] == active[3] &&
		    dbuf[9] == active[4]) {
			unlink(&dbuf[2]);
			found = 1;
		}
 
	if (found == 0) printf("Cannot find spool job no. %s\n", job);
	return;
}
 
 
itoa(ival, pbuf)
int ival;
char *pbuf;
{
	/*  convert an integer (ival) to a 6 character, null-terminated string (pbuf)  */
	char *p;
	int j, temp;
	temp = ival;
	p = pbuf+5;
	*p-- = '\0';
	for (j=1; j<6; j++) {
		*p-- = (temp%10) + '0';
		temp =/ 10;
	}
	return;
}
