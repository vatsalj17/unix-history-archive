#include <a.out.h>
#define SPACE 100		/* number of symbols read at a time */

nlist(name, list)
char *name;
struct nlist *list;
{
	register struct nlist *p, *q;
	int f, n, m, i;
	long sa;
	struct exec buf;
	struct ovlhdr ovlbuf;
	struct nlist space[SPACE];
	register int nreq = 0;

	for(p = list; p->n_name[0]; p++) {
		p->n_type = 0;
		p->n_value = 0;
		nreq++;
	}
	f = open(name, 0);
	if(f < 0)
		return(-1);
	read(f, (char *)&buf, sizeof buf);
	if(N_BADMAG(buf)){
		close(f);
		return(-1);
	}
	if (buf.a_syms == 0)
		return(1);
	sa = buf.a_text + (long)buf.a_data;
	if ( buf.a_magic == A_MAGIC5 || buf.a_magic == A_MAGIC6)
	{
		sa += sizeof ovlbuf;
		read(f, (char *)&ovlbuf, sizeof ovlbuf);
		for(i=0; i<NOVL; i++)
			sa += (long)ovlbuf.ov_siz[i];
	}
	if(buf.a_flag != 1) sa *= 2;
	sa += sizeof buf;
	lseek(f, sa, 0);
	n = buf.a_syms;

	while(n){
		m = sizeof space;
		if(n < sizeof space)
			m = n;
		read(f, (char *)space, m);
		n -= m;
		for(q = space; (m -= sizeof(struct nlist)) >= 0; q++) {
			for(p = list; p->n_name[0]; p++) {
				for(i=0;i<8;i++)
					if(p->n_name[i] != q->n_name[i]) goto cont;
				p->n_value = q->n_value;
				p->n_type = q->n_type;
				if (--nreq == 0)
					goto alldone;
				break;
		cont:		;
			}
		}
	}
alldone:
	close(f);
	return(0);
}
