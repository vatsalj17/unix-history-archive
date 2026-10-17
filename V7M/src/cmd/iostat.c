/*
 * Unix/v7m 2.1 iostat command - iostat.c
 *
 * Fred Canter 9/19/81
 *
 */

int	bflg;
int	dflg;
int	tflg;
int	iflg;
int	aflg;
int	sflg;
int	dsflag;
struct
{
	char	name[8];
	int	type;
	unsigned	value;
} nl[] = {
	"_cp_time", 0, 0,
	"_io_info", 0, 0,
	"_dk_iop", 0, 0,
	"_dk_nd", 0, 0,
	"_tk_nin", 0, 0,
	"_tk_nout", 0, 0,
	"\0\0\0\0\0\0\0\0", 0, 0
};

/*
 * WARNING !,
 * DK_NC is defined here and in /sys/h/systm.h,
 * if one is changed the other must also be changed.
 */
#define DK_NC	6	/* number of possible disk controllers */

long	cp_t[5];
long	cp_t1[5];

struct	ios
{
	char	dk_tr;		/* drive xfer rate */
	char	dk_busy;	/* drive activity flag */
	long	dk_numb;	/* drive xfer count */
	long	dk_wds;		/* drive words xfer'd (32 word clicks) */
	long	dk_time;	/* drive active time tally */
 } dk_ios[DK_NC*8], dk_ios1[DK_NC*8];

struct	ios	*dk_iop[DK_NC];	/* iostat structure pointers */
char	dk_nd[DK_NC];		/* number of drives per controller */

char	*dt[] =
{
	"hp",
	"hm",
	"hk",
	"ml",
	"rp",
	"rl",
	0
};

char	dname[10];	/* drive select name */
char	ds[7];		/* drive select numbers */

/*
 * Number of tty char's in/out.
 */

long	tin, tin1;
long	tout, tout1;

/* usec per word for the various disks */
double	xf[] = {
	0.0,	/* dummy - no dirve available */
	2.48,	/* RM02 and RP04/5/6 */
	1.65,	/* RM03 */
	4.3,	/* RK06/7 */
	1.0,	/* ML11 - 2.0  mb */
	2.0,	/* ML11 - 1.0  mb */
	4.0,	/* ML11 - 0.5  mb */
	8.0,	/* ML11 - 0.25 mb */
	7.5,	/* RP03 */
	3.9,	/* RL01/2 */
};
struct iostat {
	int	nbuf;
	long	nread;
	long	nreada;
	long	ncache;
	long	nwrite;
	long	bufcount[100];
} io_info, io_delta;
double	etime;

int	mf;

main(argc, argv)
char *argv[];
{
	struct ios *iop;
	extern char *ctime();
	register  i, j;
	int iter, lc;
	double f1, f2;
	long t;

	nlist("/unix", nl);
	for(i=0; i<4; i++)
		if(nl[i].type == -1) {
			printf("%s not found in /unix namelist\n", nl[i].name);
			exit(1);
			}
	mf = open("/dev/kmem", 0);
	if(mf < 0) {
		printf("cannot open /dev/kmem\n");
		exit(1);
	}
	iter = 0;
	while (argc>1&&argv[1][0]=='-') {
		if (argv[1][1]=='d')
			dflg++;
		else if (argv[1][1]=='s')
			sflg++;
		else if (argv[1][1]=='a')
			aflg++;
		else if (argv[1][1]=='t')
			tflg++;
		else if (argv[1][1]=='i')
			iflg++;
		else if (argv[1][1]=='b')
			bflg++;
		argc--;
		argv++;
	}
/*
 * Scan the argument list for drive select
 * specifications, up to six are allowed.
 */
	if(argc <2)
		goto dsend;
	j = 0;
dsloop:
	if((argv[1] [0] >= '0') && (argv[1] [0] <= '9'))
		goto dsend;
	dsflag++;
	dname[0] = argv[1] [0];
	dname[1] = argv[1] [1];
	dname[2] = 0;
	for(i=0; i<DK_NC; i++)
		if(equal(&dname, dt[i]))
			break;
	if(i < DK_NC) {
		ds[j] = (i * 8) + (argv[1] [2] - '0');
		if(++j >= 6)
			goto dsend;
		}
	argc--;
	argv++;
	goto dsloop;
dsend:
	if(argc > 2)
		iter = atoi(argv[2]);
/*
 * Locate and read in the pointers to the
 * iostat structures for each controller and
 * the number of drives on each controller.
 */
	lseek(mf, (long)nl[2].value, 0);
	read(mf, (char *)&dk_iop, sizeof(dk_iop));
	lseek(mf, (long)nl[3].value, 0);
	read(mf, (char *)&dk_nd, sizeof(dk_nd));
loop:
	lseek(mf, (long)nl[0].value, 0);
	read(mf, (char *)&cp_t, sizeof(cp_t));
	for(i=0; i<5; i++) {
		t = cp_t[i];
		cp_t[i] -= cp_t1[i];
		cp_t1[i] = t;
		}
	lseek(mf, (long)nl[4].value, 0);
	read(mf, (char *)&tin, sizeof(tin));
	lseek(mf, (long)nl[5].value, 0);
	read(mf, (char *)&tout, sizeof(tout));
	t = tin;
	tin -= tin1;
	tin1 = t;
	t = tout;
	tout -= tout1;
	tout1 = t;
	for(i=0; i<DK_NC; i++) {
		if(dk_iop[i] == 0)
			continue;
		iop = &dk_ios[0];
		iop += (i*8);
		lseek(mf, (long)dk_iop[i], 0);
		read(mf, (char *)iop, (sizeof(struct ios)*dk_nd[i]));
		}
	for(i=0; i<(DK_NC * 8); i++) {
		if(dsflag) {
			for(j=0; j<6; j++)
				if( i == ds[j])
					dk_ios[i].dk_busy++;
		} else
			dk_ios[i].dk_busy++;
		}
	for(i=0; i<(DK_NC * 8); i++) {
		t = dk_ios[i].dk_numb;
		dk_ios[i].dk_numb -= dk_ios1[i].dk_numb;
		dk_ios1[i].dk_numb = t;
		t = dk_ios[i].dk_wds;
		dk_ios[i].dk_wds -= dk_ios1[i].dk_wds;
		dk_ios1[i].dk_wds = t;
		t = dk_ios[i].dk_time;
		dk_ios[i].dk_time -= dk_ios1[i].dk_time;
		dk_ios1[i].dk_time = t;
		}
	if(lc == 0) {
		if (!(sflg|iflg|bflg)) {
			if (dflg) {
				long tm;
				time(&tm);
				printf("\n%s", ctime(&tm));
			}
		printf("\n");
		if(tflg)
			printf("         TTY");
		if (bflg==0) {
			for(i=0; i<(DK_NC * 8); i++)
				if(dk_ios[i].dk_tr && dk_ios[i].dk_busy)
					printf("   %s%o            ", dt[i/8], i&7);
			printf("  PERCENT\n");
			}
		if(tflg)
			printf("   tin  tout");
		if(bflg==0) {
			for(i=0; i<(DK_NC * 8); i++)
				if(dk_ios[i].dk_tr && dk_ios[i].dk_busy)
					printf("   tpm  msps  mspt");
			printf("  user  nice systm  idle\n");
			}
		}
	}
	if(++lc == 10)
		lc = 0;
	t = 0;
	for(i=0; i<4; i++)
		t += cp_t[i];
	etime = t;
	if(etime == 0.)
		etime = 1.;
	if (bflg) {
		biostats();
		goto contin;
	}
	if (sflg) {
		stats2(etime);
		goto contin;
	}
	if (iflg) {
		stats3(etime);
		goto contin;
	}
	etime /= 60.;
	if(tflg) {
		f1 = tin;
		f2 = tout;
		printf("%6.1f", f1/etime);
		printf("%6.1f", f2/etime);
	}
	for(i=0; i<(DK_NC * 8); i++)
		if(dk_ios[i].dk_tr && dk_ios[i].dk_busy)
			stats(i);
	for(i=0; i<4; i++)
		stat1(i);
	printf("\n");
	if (aflg)
		printf("%.2f minutes total\n", etime/60);
contin:
	--iter;
	if(iter)
	if(argc > 1) {
		sleep(atoi(argv[1]));
		goto loop;
	}
}


stats(dn)
{
	register i;
	double f1, f2, f3;
	double f4, f5, f6;
	long t;

	t = dk_ios[i].dk_time;
	f1 = t;
	f1 = f1/60.;
	f2 = dk_ios[i].dk_numb;
	if(f2 == 0.) {
		printf("%6.0f%6.1f%6.1f", 0.0, 0.0, 0.0);
		return;
	}
	f3 = dk_ios[i].dk_wds;
	f3 = f3*32.;
	f4 = xf[dk_ios[i].dk_tr];
	f4 = f4*1.0e-6;
	f5 = f1 - f4*f3;
	f6 = f1 - f5;
	printf("%6.0f", f2*60./etime);
	f1 = (f5*1000.)/f2;
	if(f1 < 0.)
		f1 = 0.0;
	printf("%6.1f", f1);
	printf("%6.1f", f6*1000./f2);
}

stat1(o)
{
	register i;
	long t;
	double f1, f2;

	t = 0;
	for(i=0; i<4; i++)
		t += cp_t[i];
	f1 = t;
	if(f1 == 0.)
		f1 = 1.;
	f2 = cp_t[o];
	printf("%6.2f", f2*100./f1);
}

stats2(t)
double t;
{
	register i, j, k;

	if (dflg) {
		long tm;
		time(&tm);
		printf("\n\n%s", ctime(&tm));
	}
	printf("\ndisk\tdrive\tus/word\t%%time\txfer's\t  words\n");
	for (i=0; i<DK_NC; i++) {
		for (j=0; j<8; j++) {
			k = (i*8) + j;
			if(dk_ios[k].dk_tr == 0)
				continue;
			printf("\n%s\t%d\t", dt[i], j);
			printf("%4.2f\t", xf[dk_ios[k].dk_tr]);
			printf("%5.2f\t", dk_ios[i*8+j].dk_time/(t/100));
			printf("%D\t  ", dk_ios[k].dk_numb);
			printf("%D", (dk_ios[k].dk_wds)*32);
			}
	}
		printf("\n");
	if (aflg)
		printf("\n%.2f minutes total\n", etime/3600);
}

stats3(t)
double t;
{
	register i, j;
	double sum;

	if (dflg) {
		long tm;
		time(&tm);
		printf("\n\n%s", ctime(&tm));
	}
	printf("\n %%time state\n");
	t /= 100;
	sum = cp_t[3];
	printf("%6.2f idle\n", sum/t);
	sum = cp_t[0];
	printf("%6.2f user\n", sum/t);
	sum = cp_t[1];
	printf("%6.2f nice\n", sum/t);
	sum = cp_t[2];
	printf("%6.2f system\n", sum/t);
	sum = cp_t[4];
	printf("%6.2f IO wait\n", sum/t);
	sum = 0;
	for (i=0; i<(DK_NC * 8); i++)
		if(dk_ios[i].dk_tr)
			sum += dk_ios[i].dk_time;
	printf("%6.2f IO active\n", sum/t);
	for(i=0; i<DK_NC; i++) {
		sum = 0;
		for(j=0; j<8; j++)
			sum += dk_ios[(i*8)+j].dk_time;
		if(sum)
			printf("%6.2f %s active\n", sum/t, dt[i]);
		}
	if (aflg)
		printf("\n%.2f minutes total\n", etime/3600);
}

biostats()
{
register i;

	if (dflg) {
		long tm;
		time(&tm);
		printf("\n%s", ctime(&tm));
	}
	lseek(mf,(long)nl[1].value, 0);
	read(mf, (char *)&io_info, sizeof(io_info));
	printf("\nnbuf\tnread\tnreada\tncache\tnwrite\n");
	printf("%d\t%D\t%D\t%D\t%D\n",
	 io_info.nbuf,
	 io_info.nread-io_delta.nread, io_info.nreada-io_delta.nreada,
	 io_info.ncache-io_delta.ncache, io_info.nwrite-io_delta.nwrite);

	printf("\nI/O operations per buffer (0->NBUF)\n");
	for(i=0; i<io_info.nbuf; ) {
		printf("%D\t",(long)io_info.bufcount[i]-io_delta.bufcount[i]);
		i++;
		if (i % 10 == 0)
			printf("\n");
	}
	io_delta = io_info;
	printf("\n");
	if (aflg)
		printf("\n%.2f minutes total\n", etime/3600);
}

equal(a, b)
char *a, *b;
{
	return(!strcmp(a, b));
}
