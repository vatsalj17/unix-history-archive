#define LSIR 1
#define LSIW 0
#define NKL 53
#define MIN(a,b)  a>b?b:a
#define STX 002
#define DLE 020
#define ETB 046
#define ETX 003
#define IRS 036

/* /*
   //* error in transmission
   /*route  print cancel
*/

char kill[] {
STX,0141,0134,IRS,
0141,0141,0134,0100,0305,0331,0331,0326,0331,0100,0311,0325,0100,0343,
0331,0301,0325,0342,0324,0311,0342,0342,0311,0326,0325,IRS,
0141,0134,0331,0326,0344,0343,0305,0100,0100,0327,0331,0311,0325,0343,
0100,0303,0301,0325,0303,0305,0323,IRS,ETX
};
char ibuf[512];
char xbuf[514];
char *obuf &xbuf[2];
char *p, *b, *ptr;
int colcnt, bcnt, cnt, minum, wdone;

main(argc,argv) char *argv;
{
	int jj;

	if (open(*argv,0) != 1) exit(4);
	p = obuf;

/* b is ptr to ibuf; bcnt is bytes left in ibuf;
   p is ptr to obuf; colcnt is column position in line */

	bcnt = 0; *p++ = STX;
	for(;;)

		/* while chars in buffer process them */

		if (--bcnt >= 0) {

			/* get line length and check bounds */

			if (((colcnt = *b++) < 0) || (colcnt > 80))
				exit(bad());

			/* room? if not end block, flush block, continue */

		nmode:  if ((p + colcnt) > (obuf + 510)) {
				*p++ = ETB;
				if (jj=_flush(0,p)) exit(jj);
				else p = obuf + 1;
			}

			/* save ptr and cnt in case xmode char hit */

			ptr = p; cnt = colcnt;

			/* process line - may have to replenish buffer
			   if line crosses buffers */

			while (colcnt > 0) {
				if ((bcnt <= 0) && (bcnt = rdunix()) < 0)
					exit(bad());
				minum = MIN(bcnt,colcnt);
				bcnt =- minum; colcnt =- minum;

				/* process chars - see if xmode */

				do {
					if ((*b & 0300) == 0)
						if ((jj = xmode()) != 0) exit(jj);
						else goto nmode;
					else *p++ = *b++;
				} while(--minum > 0);
			}

			/* put record  terminator and continue */

			*p++ = IRS;
		}
		else

			/* buffer empty */

			if ((bcnt = rdunix()) <= 0) {
				*p++ = ETX;
				if (jj = _flush(0,p)) exit(jj);
				else exit(_fin());
			}
	exit(1);
}

xmode()
{
	register  char *sptr;
	register int dcnt, blnk;
	char savc;
	int jj, sdcnt;

	/* put rest of chars from line not processed
	   by main as xmode char hit  */

	do
		*p++ = *b++;
	while (--minum > 0);

	/* save first char of this line */

	savc = *ptr;

	/* put ETB terminating all prev lines in buffer */

	*ptr = ETB;

	/* flush buffer */

	if (jj = _flush(0,ptr + 1)) return(jj);

	/* restore char */

	*ptr = savc;
	sptr = ptr;

	/* reset ptr to top, calc chars in line */

	ptr = &xbuf; sdcnt = dcnt = p - sptr;

	/* put header */

	*ptr++ = DLE; *ptr++ = STX;

	/* move line to top of buffer */

	do
		*ptr++ = *sptr++;
	while (--dcnt > 0);

	/* calc blank count, etc */

	blnk = 80 - cnt; colcnt = cnt - sdcnt; p = ptr;
	goto xlp2;

	/* process lines */

	for(;;) {
		if (--bcnt < 0) {
			if ((bcnt = rdunix()) <= 0) break;
			else continue;
		}
		if (((colcnt = *b++) < 0) || ((blnk = 80 - colcnt) < 0))
			return(bad());
		if ((xbuf + 430) < p) {
			*p++ = DLE; *p++ = ETB;
			if (jj = _flush(1,p)) return(jj);
			else {
				p = obuf;
				*p++ = STX;
				return(0);
			}
		}
	xlp2:
		while (colcnt > 0) {
			if ((bcnt <= 0) && ((bcnt = rdunix()) <= 0))
				return(bad());
			minum = MIN(bcnt,colcnt);
			bcnt =- minum; colcnt =- minum;
			do
				*p++ = *b++;
			while (--minum > 0);
		}

		/* pad line to length of 80 */

		if (blnk > 0)
			do
				*p++ = 0100;
			while (--blnk > 0);
	}
	*p++ = DLE; *p++ = ETX;
	if (jj = _flush(1,p)) return(jj);
	else return(_fin());
}

_fin()
{
	if (write(LSIW,0,0) < 0) return(err());
	else return(1);
}

bad()
{
	if (wdone > 0) {
		write(LSIW,kill,NKL);
		write(LSIW,0,0);
	}
	return(5);
}

_flush(i,eptr) int i; char *eptr;
{ register int mcnt, j;
  register char *ptrb;
  int wcnt;
	if (i) {
		ptrb = xbuf;
		mcnt = 4;
	}
	else {
		ptrb = obuf;
		mcnt = 2;
	}
	wcnt = eptr - ptrb;
	if (wcnt > mcnt) {
		j = write(LSIW,ptrb,wcnt);
		if (j < 0) return(err());
		else if (j == 0) return(2);
		else wdone = 1;
	}
	return(0);
}

err()
{ register int j, repcnt;
	repcnt = 5;
	do
		j = write(LSIW,kill,NKL);
	while (--repcnt && j <= 0);
	write(LSIW,0,0);
	if (repcnt <= 0) return(3);
	else return(0);
}

rdunix()
{
	b = ibuf;
	return(read(LSIR,ibuf,512));
}

