#include <sys/label.h>

#define NPRIVS (sizeof(privs)-1)

static char privs[] = "guxnlp";
static char frigs[] = " FRC";
static char flags[] = "UYN ";

static char buf[4+2*NPRIVS+2*LABSIZ+LABSIZ/2];
static char pbuf[NPRIVS+1];

char *
privtoa(v)
{
	int i;
	for(i=0; i<NPRIVS; i++)
		pbuf[i] = (v>>(NPRIVS-1-i))&1? privs[i]: '-';
	return pbuf;
}

static hex(n,k)
{
	buf[k] = n<10? n+'0': n-10+'a';
}

char *labtoa(p)
struct label *p;
{
	register i, j, k=0;
	char *pbuf = privtoa(p->lb_t);
	for(i=0; i<NPRIVS; i++)
		buf[k++] = pbuf[i];
	buf[k++] = ' ';
	pbuf = privtoa(p->lb_u);
	for(i=0; i<NPRIVS; i++)
		buf[k++] = pbuf[i];
	buf[k++] = ' ';
	buf[k++] = frigs[p->lb_fix];
	buf[k++] = flags[p->lb_flag];
	for(j=LABSIZ; j>1; j--)
		if(p->lb_bits[j-1] != p->lb_bits[j-2])
			break;
	j = (j+1)/2*2;
	if(j < LABSIZ)
		j += 2;
	for(i=0; i<j; i+=2) {
		buf[k++] = ' ';
		hex((p->lb_bits[i]>>4)&017,k++);
		hex(p->lb_bits[i]&017,k++);
		hex((p->lb_bits[i+1]>>4)&017,k++);
		hex(p->lb_bits[i+1]&017,k++);
	}
	if(j < LABSIZ) {
		buf[k++] = ' ';
		buf[k++] = '.';
		buf[k++] = '.';
		buf[k++] = '.';
	}
	buf[k] = 0;
	return buf;
}
