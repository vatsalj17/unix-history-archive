long	itol();
struct
{
	int	high;
	int	low;
};
struct nlist
{
	int	name[4]; /* really char[8] */
	int	type;
	int	value;
};

nlist(name, list)
char *name;
struct nlist *list;
{
	register struct nlist *p, *q;
	int f, n, m;
	long lng;
	int buf[010];
	struct nlist space[100];

	for(p = list; p->name[0]; p++) {
		p->type = 0;
		p->value = 0;
	}
	f = open(name, 0);
	if(f < 0)
		return(-1);
	read(f, buf, sizeof buf);
	n = buf[0];
	if(n != 0407 && n != 0410 && n != 0411) {
		close(f);
		return(-1);
	}
	lng = itol(0, buf[1]);
	lng =+ itol(0, buf[2]);
	if(buf[7] != 1)
		lng = lng << 1;
	lng =+ 020;
	lseek(f, lng, 0);
	n = buf[4];

loop:
	if(n == 0) {
		close(f);
		return(0);
	}
	m = 1200;
	if(n < 1200)
		m = n;
	read(f, space, m);
	n =- m;
	for(q = space; (m =- 12) >= 0; q++) {
		for(p = list; p->name[0]; p++) {
			if(p->name[0] != q->name[0])
				continue;
			if(p->name[1] != q->name[1])
				continue;
			if(p->name[2] != q->name[2])
				continue;
			if(p->name[3] != q->name[3])
				continue;
			p->value = q->value;
			p->type = q->type;
			break;
		}
	}
	goto loop;
}
