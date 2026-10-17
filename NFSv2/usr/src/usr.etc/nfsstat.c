#ifndef lint
static  char sccsid[] = "@(#)nfsstat.c 1.1 85/05/31 Copyr 1983 Sun Micro";
#endif
/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

/* 
 * nfsstat: Network File System statistics
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#ifdef sun
#include <sun/ndio.h>
#endif
#include <nlist.h>

struct nlist nl[] = {
#define	X_RCSTAT	0
	{ "_rcstat" },
#define	X_CLSTAT	1
	{ "_clstat" },
#define	X_RSSTAT	2
	{ "_rsstat" },
#define	X_SVSTAT	3
	{ "_svstat" },
#ifdef sun
#define	X_NDSTAT	4
	{ "_ndstat" },
#endif
	"",
};

int kmem;	/* file descriptor for /dev/kmem */

/*
 * client side rpc statistics
 */
struct {
        int     rccalls;
        int     rcbadcalls;
        int     rcretrans;
        int     rcbadxids;
        int     rctimeouts;
        int     rcwaits;
        int     rcnewcreds;
} rcstat;

/*
 * client side nfs statistics
 */
struct {
        int     nclsleeps;              /* client handle waits */
        int     nclgets;                /* client handle gets */
        int     ncalls;                 /* client requests */
        int     nbadcalls;              /* rpc failures */
        int     reqs[32];               /* count of each request */
} clstat;

/*
 * Server side rpc statistics
 */
struct {
        int     rscalls;
        int     rsbadcalls;
        int     rsnullrecv;
        int     rsbadlen;
        int     rsxdrcall;
} rsstat;

/*
 * server side nfs statistics
 */
struct {
        int     ncalls;         /* number of calls received */
        int     nbadcalls;      /* calls that failed */
        int     reqs[32];       /* count for each request */
} svstat;

#ifdef sun
struct ndstat ndstat;
#endif

main(argc, argv)
	char *argv[];
{
	char *options;
	int	cflag = 0;		/* client stats */
	int	dflag = 0;		/* network disk stats */
	int	nflag = 0;		/* nfs stats */
	int	rflag = 0;		/* rpc stats */
	int	sflag = 0;		/* server stats */
	int	zflag = 0;		/* zero stats after printing */


	if (argc > 2) {
		usage();
	}
	if (argc == 2) {
		if (*argv[1] != '-') {
			usage();
		}
		options = &argv[1][1];
		while (*options) {
			switch (*options) {
			case 'c':
				cflag++;
				break;
			case 'd':
				dflag++;
				break;
			case 'n':
				nflag++;
				break;
			case 'r':
				rflag++;
				break;
			case 's':
				sflag++;
				break;
			case 'z':
				if (getuid()) {
					fprintf(stderr,
					    "Must be root for z flag\n");
					exit(1);
				}
				zflag++;
				break;
			default:
				usage();
			}
			options++;
		}
	}

	setup(zflag);
	getstats();
	if (dflag || (dflag + cflag + sflag + nflag + rflag) == 0) {
		d_print(zflag);
	}
	if (dflag && (sflag + cflag + rflag + nflag) == 0) {
		if (zflag) {
			putstats();
		}
		exit(0);
	}
	if (sflag || (!sflag && !cflag)) {
		if (rflag || (!rflag && !nflag)) {
			sr_print(zflag);
		}
		if (nflag || (!rflag && !nflag)) {
			sn_print(zflag);
		}
	}
	if (cflag || (!sflag && !cflag)) {
		if (rflag || (!rflag && !nflag)) {
			cr_print(zflag);
		}
		if (nflag || (!rflag && !nflag)) {
			cn_print(zflag);
		}
	}
	if (zflag) {
		putstats();
	}
}

getstats()
{
	int size;

	if (lseek(kmem, (long)nl[X_RCSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
	if (read(kmem, &rcstat, sizeof rcstat) != sizeof rcstat) {
		fprintf(stderr, "can't read rcstat from kmem\n");
		exit(1);
	}

	if (lseek(kmem, (long)nl[X_CLSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, &clstat, sizeof(clstat)) != sizeof (clstat)) {
		fprintf(stderr, "can't read clstat from kmem\n");
		exit(1);
	}

	if (lseek(kmem, (long)nl[X_RSSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, &rsstat, sizeof(rsstat)) != sizeof (rsstat)) {
		fprintf(stderr, "can't read rsstat from kmem\n");
		exit(1);
	}

	if (lseek(kmem, (long)nl[X_SVSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, &svstat, sizeof(svstat)) != sizeof (svstat)) {
		fprintf(stderr, "can't read svstat from kmem\n");
		exit(1);
	}

#ifdef sun
	if (lseek(kmem, (long)nl[X_NDSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);

	}
 	if (read(kmem, &ndstat, sizeof(ndstat)) != sizeof (ndstat)) {
		fprintf(stderr, "can't read ndstat from kmem\n");
		exit(1);
	}
#endif
}

putstats()
{
	if (lseek(kmem, (long)nl[X_RCSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
	if (write(kmem, &rcstat, sizeof rcstat) != sizeof rcstat) {
		fprintf(stderr, "can't write rcstat to kmem\n");
		exit(1);
	}

	if (lseek(kmem, (long)nl[X_CLSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (write(kmem, &clstat, sizeof(clstat)) != sizeof (clstat)) {
		fprintf(stderr, "can't write clstat to kmem\n");
		exit(1);
	}

	if (lseek(kmem, (long)nl[X_RSSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (write(kmem, &rsstat, sizeof(rsstat)) != sizeof (rsstat)) {
		fprintf(stderr, "can't write rsstat to kmem\n");
		exit(1);
	}

	if (lseek(kmem, (long)nl[X_SVSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (write(kmem, &svstat, sizeof(svstat)) != sizeof (svstat)) {
		fprintf(stderr, "can't write svstat to kmem\n");
		exit(1);
	}

#ifdef sun
	if (lseek(kmem, (long)nl[X_NDSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}

 	if (write(kmem, &ndstat, sizeof(ndstat)) != sizeof (ndstat)) {
		fprintf(stderr, "can't write ndstat to kmem\n");
		exit(1);
	}
#endif
}

setup(zflag)
	int zflag;
{
	nlist("/vmunix", nl);
	if (nl[0].n_value == 0) {
		fprintf (stderr, "Variables missing from namelist\n");
		exit (1);
	}
	if ((kmem = open("/dev/kmem", zflag ? 2 : 0)) < 0) {
		fprintf(stderr, "can't open kmem\n");
		exit(1);
	}
}

cr_print(zflag)
	int zflag;
{
	fprintf(stdout, "\nClient rpc:\n");
	fprintf(stdout,
	 "calls      badcalls   retrans    badxid     timeout    wait       newcred\n");
	fprintf(stdout,
	    "%-11d%-11d%-11d%-11d%-11d%-11d%-11d\n",
	    rcstat.rccalls,
            rcstat.rcbadcalls,
            rcstat.rcretrans,
            rcstat.rcbadxids,
            rcstat.rctimeouts,
            rcstat.rcwaits,
            rcstat.rcnewcreds);
	if (zflag) {
		bzero(&rcstat, sizeof rcstat);
	}
}

sr_print(zflag)
	int zflag;
{
	fprintf(stdout, "\nServer rpc:\n");
	fprintf(stdout,
	    "calls      badcalls   nullrecv   badlen     xdrcall\n");
	fprintf(stdout,
	    "%-11d%-11d%-11d%-11d%-11d\n",
           rsstat.rscalls,
           rsstat.rsbadcalls,
           rsstat.rsnullrecv,
           rsstat.rsbadlen,
           rsstat.rsxdrcall);
	if (zflag) {
		bzero(&rsstat, sizeof rsstat);
	}
}

#define RFS_NPROC       18
char *nfsstr[RFS_NPROC] = {
	"null",
	"getattr",
	"setattr",
	"root",
	"lookup",
	"readlink",
	"read",
	"wrcache",
	"write",
	"create",
	"remove",
	"rename",
	"link",
	"symlink",
	"mkdir",
	"rmdir",
	"readdir",
	"fsstat" };

cn_print(zflag)
	int zflag;
{
	int i;

	fprintf(stdout, "\nClient nfs:\n");
	fprintf(stdout,
	    "calls      badcalls   nclget     nclsleep\n");
	fprintf(stdout,
	    "%-11d%-11d%-11d%-11d\n",
            clstat.ncalls,
            clstat.nbadcalls,
            clstat.nclgets,
            clstat.nclsleeps);
	req_print((int *)clstat.reqs, clstat.ncalls);
	if (zflag) {
		bzero(&clstat, sizeof clstat);
	}
}

sn_print(zflag)
	int zflag;
{
	fprintf(stdout, "\nServer nfs:\n");
	fprintf(stdout, "calls      badcalls\n");
	fprintf(stdout, "%-11d%-11d\n", svstat.ncalls, svstat.nbadcalls);
	req_print((int *)svstat.reqs, svstat.ncalls);
	if (zflag) {
		bzero(&svstat, sizeof svstat);
	}
}

d_print(zflag)
	int zflag;
{
#ifdef sun
	fprintf(stdout, "\nNetwork Disk:\n");
	fprintf(stdout, "rcv %d snd %d retrans %d (%.2f%%)\n",
	    ndstat.ns_rpacks,ndstat.ns_xpacks,ndstat.ns_rexmits,
	    (double)(ndstat.ns_rexmits*100)/ndstat.ns_xpacks);
	if (zflag) {
		bzero(&ndstat, sizeof ndstat);
	}
#endif
}

req_print(req, tot)
	int	*req;
	int	tot;
{
	int	i, j;
	char	fixlen[128];

	for (i=0; i<=RFS_NPROC / 7; i++) {
		for (j=i*7; j<min(i*7+7, RFS_NPROC); j++) {
			fprintf(stdout, "%-11s", nfsstr[j]);
		}
		fprintf(stdout, "\n");
		for (j=i*7; j<min(i*7+7, RFS_NPROC); j++) {
			if (tot) {
				sprintf(fixlen,
				    "%d %2d%% ", req[j], (req[j]*100)/tot);
			} else {
				sprintf(fixlen, "%d 0%% ", req[j]);
			}
			fprintf(stdout, "%-11s", fixlen);
		}
		fprintf(stdout, "\n");
	}
}

usage()
{
	fprintf(stderr, "nfsstat [-cdnrsz]\n");
	exit(1);
}

min(a,b)
	int a,b;
{
	if (a<b) {
		return(a);
	}
	return(b);
}
