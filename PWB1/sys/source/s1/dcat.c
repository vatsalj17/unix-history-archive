/*
	dcat -- ``cat'' to or from dial du11 pr dp11

	note:	timing delays expressed are a mixture of experience
		and superstition.

		each block is hashed, but total copy is not.
*/
#include "stdio.h"
#define DN	"/dev/dn0"
#define ACK	"\006\006\006\006"
#define NAK	"\025\025\025\025"
#define BEL	"\007\007\007\007"
#define SYN	"\026\026\026\026"
#define N_SYN	4
#define ALARM	14
#define MAXERR	10
char Du[] = "/dev/du0";

struct Buf {
	char	syn[N_SYN];
	char	stuff[508];
} Buf[2];

struct Msg {
	int	seq,
		count,
		hash;
	char	buf[400];
} Msg = { 0, 0, 0 };

int	Dev,
	Errors = 0;
char	Resp[520];
extern	errno;

sigalrm()
{
	signal(ALARM, sigalrm);
}

main(argc, argv)
char **argv;
{
	if( argc != 2 || argv[1][0] != '-' )
		showusage();
	strcpy(&Buf[0], SYN); strcpy(&Buf[1], SYN);

	switch(argv[1][1]) {
	case 't':	/* Transmit */
		ckdial(&argv[1][2]);
		writedu();
		break;
	case 'r':	/* Receive */
		ckdial(&argv[1][2]);
		readdu();
		break;
	default:
		showusage();
	}
	exit(0);
}

showusage()
{
	fprintf(stderr, "Usage: dcat {-r | -t}<telno>\n");
	exit(1);
}

ckdial(arg)
char *arg;
{
	char	telno[16];
	int	dn;

	if(!*arg) {
		opendu(10);
		return;
	}
	opendu(7);
	strcpy(telno, arg);
	telno[strlen(telno)] = '-';
	telno[strlen(telno)] = '=';
	if((dn = open(DN, 1)) < 0) {
		fprintf(stderr, "dn busy\n");
		exit(1);
	}
	if(write(dn, telno, strlen(telno)) < 0) {
		fprintf(stderr, "dn write failed (%d)\n", errno);
		exit(1);
	}
	close(dn);
	sleep(10);
}

opendu(n)
{
	int attempt = 0;

	if(access(Du, 06) < 0)
		Du[6] = 'p';  /* no du11 -- try dp11 */
	while((Dev = open(Du, 2)) < 0) {
		if( ++attempt >= 5 ) {
			fprintf(stderr, "cannot open du\n");
			exit(1);
		} else {
			sleep(5);
		}
	}
	sleep(n); /* long enough for the called phone to ring */
}

writedu()
{
	register i, hashit;
	register char *bp;
	int ct;
	int out_of_seq = 0;

	while((Msg.count = read(0, Msg.buf, sizeof Msg.buf)) > 0) {
		hashit = 0;
		bp = Msg.buf;
		for(i = 0; i < Msg.count; ++i)
			hashit += *bp++;
		Msg.hash = hashit;
		ct = _8to7(&Msg, Buf[Msg.seq].stuff, Msg.count + 6);
resend:
		for(;;) {
			sigalrm();
			alarm(10);
			if(write(Dev, &Buf[Msg.seq], ct+N_SYN+2) < 0) {
				if(Errors++ > MAXERR) {
					fprintf(stderr, "write fail (%d)\n",
						errno);
					exit(1);
				}
				sleep(5); /* in case write fails immediately */
				continue;
			}
			alarm(0);
			while(read(Dev, Resp, 512) < 0) {
				if(Errors++ > MAXERR) {
					fprintf(stderr, "cannot read reply\n");
					exit(1);
				}
			}
			for(bp = Resp; *bp == SYN[0]; bp++);
			if (bp[0] == ACK[0]) /* ACK respose */
				break;
			if(Errors++ >= MAXERR) {
				fprintf(stderr, "Trans incomplete--errors\n");
				exit(1);
			}
			if(bp[0] == BEL[0]) { /* out-of-seq */
				if(Errors++ >= MAXERR) {
					fprintf(stderr, "excess errors\n");
					exit(1);
				}
				Msg.seq = Msg.seq? 0 : 1;
				out_of_seq = 1;
			}
		}
		Msg.seq = Msg.seq? 0: 1;
		if(out_of_seq) {
			out_of_seq = 0;
			goto resend;
		}
		if(Errors)
			--Errors;
	}
}

readdu()
{
	register i, hashck;
	register char *bp;
	int ct, last_seq = 1, attempt, good = 0;

	for(attempt=1; attempt < 10;attempt++) {
		while((ct = read(Dev, &Buf[0].stuff, 512)) > 0) {
			if(ct == 0) /* for drivers that time-out */
				continue;
			good = 1; /* first good(?) read */
			for(bp = Buf[0].stuff; *bp == SYN[0]; ++bp);
			_7to8(bp, &Msg, ct);
			if(Msg.seq == last_seq) {
				fprintf(stderr, "sequence error\n");
				ans(BEL); /* BEL used as seq NAK */
				continue;
			}
			hashck = 0;
			bp = Msg.buf;
			for(i = 0; i < Msg.count; ++i)
				hashck += *bp++;
			if(hashck != Msg.hash) {
				if(Errors++ >= MAXERR) {
					fprintf(stderr,
						"Receive incomplete\n");
					exit(1);
				}
				ans(NAK);
				continue;
			}
			ans(ACK);
			Errors = 0;
			last_seq = last_seq? 0: 1;
			write(1, Msg.buf, Msg.count);
		}
		if(good)
			return;
		fprintf(stderr,"du read fail# %d(%d)\n", attempt, errno);
		sleep(10);
	}
	fprintf(stderr, "Excessive re-tries\n");
	exit(1);
}

ans(resp)
char *resp;
{
	char buf[32];

if(*resp != ACK[0]) fprintf(stderr, "NAK\n\r");
	strcpy(buf, SYN);
	strcat(buf, resp);
	if(write(Dev, buf, strlen(buf)) < 0) {
		fprintf(stderr, "cannot ack/nak\n\r");
		exit(1);
	}
}

_8to7(ip, op, n)
char *ip, *op;
int n;
{
	register int temp, i, j, k = 0;

	while (n > 0) {
		i = 7 > n? n: 7;
		k += 8;
		op[7] = 0;
		for (j = 0; j < i; j++) {
			--n;
			temp = *ip++ & 0377;
			op[j]	= temp & 0177;
			op[7] =| (temp & 0200) >> (7 - j);
		}
		op = &op[8];
	}
	return k;
}


_7to8(ip, op, n)
char *ip, *op;
int n;
{
	register int i, j;

	for (; n > 0; n =- 8) {
		for (j = 0; j < 7; j++)
			op[j] = *ip++;
		for (j = 0; j < 7; j++) {
			i = (((*ip & 0177) << (7 - j)) & 0200);
			op[j] =| i;
		}
		ip++;
		op = &op[7];
	}
}
