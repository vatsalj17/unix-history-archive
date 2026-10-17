#
/*
 *		Line printer daemon
 */

/* File names */
char lpd[]	"/usr/lpd";
char lock[]	"/usr/lpd/lock";
char printer[]	"/dev/pr";

/*
 * control sequence to set unix default tabs on hydra
 */
char stabs[] {
	"\033G008016024032040048056064072080088096104112120128\n"
};
/* Buffers */
int cmdbuf[131];	/* for reading command file */
char prbuf[512];	/* for copying file to printer */
char lbuf[64];		/* for assembling lines from command file */
char dbuf[17];		/* for reading directory */

/* Tty modes for printer */
int ttymode[3] {
	(11<<8) | 11,		/* 2400 baud */
	0,
	020			/* CRMOD */
};

main()
{
	register flag;

	/* Ignore quit, interrupt, hangup */
	signal(1, 1);
	signal(2, 1);
	signal(3, 1);

	/* Use lock to prevent multiple active daemons */
	if (creat(lock, 0) < 0)
		exit(0);

	/* Fork and exit, thus spawning the daemon */
	if (fork())
		exit(0);

	/* Before opening the printer, make sure there's something to print */
	close(0);
	close(1);
	close(2);
	if (open(lpd, 0) != 0)
		dexit();
	flag = 0;
	while (read(0, dbuf, 16) == 16)
		if ((dbuf[0] | dbuf[1]) != 0
		    && dbuf[2] == 'd' && dbuf[3] == 'f')
			flag++;
	if (!flag)
		dexit();

	/* Open the printer, and set proper tty modes */
	if (open(printer, 1) != 1)
		dexit();
	stty(1, ttymode);
	/* set hydra tabs */
	write(1, stabs, sizeof stabs);	/*** hydra ***/

	/* Search lpd directory for work to do */
	if (chdir(lpd) < 0)
		dexit();
	seek(0, 0, 0);
	while (read(0, dbuf, 16) == 16) {
		if ((dbuf[0] | dbuf[1]) == 0
		    || dbuf[2] != 'd' || dbuf[3] != 'f')
			continue;

		/* Process one command file */
		spool(&dbuf[2]);
		unlink(&dbuf[2]);

		/* Reread directory in case it changed while we were spooling */
		seek(0, 0, 0);
	}

	/* Feed paper partly out of printer */
	write(1, "\033H44\n", 5);

	dexit();
}

/*
 * Remove lock and terminate
 */
dexit()
{
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

	if (fopen(dfname, cmdbuf) < 0)
		return;

	for(;;) {
		switch (c = getc(cmdbuf)) {
	
		/* EOF or unknown command */
		default:
			close(cmdbuf[0]);
			return;
	
		/* Unsupported commands */
		case 'S':
		case 'M':
			getline(lbuf);
			continue;
	
		/* header page */
		case 'L':
			lbuf[0] = lbuf[1] = ' ';
			getline(&lbuf[2]);
			header();
			continue;

		/* Copy file in binary ('F' means prepend a form feed */
		case 'F':
		case 'B':
			getline(lbuf);
			if ((in = open(lbuf, 0)) < 0)
				continue;
			while ((len = read(in, prbuf, 512)) > 0
			    && write(1, prbuf, len) == len)
				;
			if (c == 'F')
				write(1, "\014", 1);
			close(in);
			continue;
	
		/* Delete file after copying */
		case 'U':
			getline(lbuf);
			unlink(lbuf);
			continue;
		}
	}
	close(cmdbuf[0]);
}


/*
 * Read rest of line from command file
 */
getline(bufp)
char *bufp;
{
	register char *p;
	register c;

	for (p = bufp; (*p = c = getc(cmdbuf)) != '\n' && c > 0; p++)
		;
	*p = '\0';
}

/*
 * Print header page
 */

char hd0[] =
"========== University of Wollongong ========================\
==================== Computing Science Laboratory ==========\n\n\n";
char hd1[] =
"\n\n\n=============================================== ";
char hd2[] =
" ===============================================\n\014";

header()
{
	int tbuf[2];

	time(tbuf);
	write(1, hd0, sizeof hd0);
	bprint(lbuf);
	write(1, hd1, sizeof hd1);
	write(1, ctime(tbuf), 24);
	write(1, hd2, sizeof hd2);
}
