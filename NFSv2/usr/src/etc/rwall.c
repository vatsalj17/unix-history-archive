#ifndef lint
static  char sccsid[] = "@(#)rwall.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <utmp.h>
#include <rpc/rpc.h>
#include <rpcsvc/netwall.h>

#define	USERS	128
char who[9] = "???";
char *path;

main(argc, argv)
	char **argv;
{
	int msize;
	char buf[BUFSIZ];
	register i;
	struct	utmp utmp[USERS];
	FILE *f;
	int sline;
	char	hostname[256];
	int hflag;
	char *machine, *user, *domain;
	
	if (argc < 2)
		usage();
	gethostname(hostname, sizeof (hostname));

	if((f = fopen("/etc/utmp", "r")) == NULL) {
		fprintf(stderr, "Cannot open /etc/utmp\n");
		exit(1);
	}
	sline = ttyslot(2); /* 'utmp' slot no. of sender */
	fread((char *)utmp, sizeof(struct utmp), USERS, f);
	fclose(f);
	if (sline)
		strncpy(who, utmp[sline].ut_name, sizeof(utmp[sline].ut_name));

	sprintf(buf, "broadcast message from %s!%s:  ", hostname, who);
	msize = strlen(buf);
	while((i = getchar()) != EOF) {
		if (msize >= sizeof buf) {
			fprintf(stderr, "Message too long\n");
			exit(1);
		}
		buf[msize++] = i;
	}
	path = buf;
	
	hflag = 1;
	while(argc > 1) {
		if (argv[1][0] == '-') {
			switch (argv[1][1]) {
				case 'h':
					hflag = 1;
					break;
				case 'n':
					hflag = 0;
					break;
				default:
					usage();
					break;
			}
		}
		else if (hflag)
			doit(argv[1]);
		else {
			setnetgrent(argv[1]);
			while (getnetgrent(&machine, &user, &domain))
				doit(machine);
			endnetgrent();
		}
		argc--;
		argv++;
	}
}

doit(host)
	char *host;
{
	int errs;
	
	errs = callrpc(host, WALLPROG, WALLVERS,
	    WALLPROC_WALL, xdr_wrapstring, &path, xdr_void, 0);
	if (errs) {
		fprintf(stderr, "%s: ", host);
		clnt_perrno(errs);
		fprintf(stderr, "\n");
	}
}

usage()
{
	fprintf(stderr,
	    "Usage: rwall host .... [-n netgroup ....] [-h host ...]\n");
	exit(1);
}
