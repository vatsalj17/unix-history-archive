/*
 * Join relations module
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>

#define	DISP	"dispatch"
#define	NVEC	512
#define	NBITS	32
#define	B(b)	(1 << (b))
#define	Z	((char *) 0)

typedef struct {
	char	*attr, *val;
	int	a;
} query_t;

typedef struct {
	int	attrs, global, na;
	char	*pq, *args;
} rel_t;

typedef struct {
	int	attrs, wrote;
	char	*base;
	rel_t	*rp;
} merge_t;

typedef struct {
	int	attrs, list, argc;
	char	*pq, *base;
	rel_t	*rp;
	query_t	query[NVEC];
	merge_t	merge[NBITS];
	rel_t	rel[NBITS];
	char	*attr[NBITS];
	char	buf[5120];
} join_t;

static char	*mpqopen();
static int	pqowrite(), key();
extern char	*pq_open(), *path(), *strcon();

char *
join_open(argv)
char	*argv[];
{
	join_t	*join = (join_t *) malloc(sizeof (join_t));
	rel_t	*rp = join->rel, *dp = 0;
	int	na = 0, dups = 0, fd;
	char	*str, *nl, *dup;

	if ((fd = open(str = path(argv[0] ? argv[0] : DISP), O_RDONLY)) < 0) {
		error(-1, "join %s", str);
		free((char *) join);
		return 0;
	}
	*(join->base = join->buf + read(fd, join->buf, sizeof join->buf)) = 0;
	close(fd);

	if (*join->buf != '>') {
		if ((join->pq = mpqopen(join->buf)) == 0) {
			free((char *) join);
			return 0;
		}
		return (char *) join;
	}
	join->pq = 0;
	join->rp = 0;
	join->list = 0;
	join->attrs = 0;

	for (str = join->buf; nl = strchr(str, '\n'); str = ++nl)
		if (*str == '>') {
			*str++ = *nl = 0;
			dup = strchr(str, '/');
			rp->na = strvec(str, join->attr + na, "/ \t\n");
			for (rp->attrs = 0; join->attr[na]; na++) {
				rp->attrs |= B(na);
				if (dup && join->attr[na] > dup)
					dups |= B(na);
			}
			if (rp->na > 1) {
				rp->global = rp->attrs & ~dups;
				join->attrs |= B(key(rp->attrs)) | dups;
			} else if (rp->na == 0)
				dp = rp;
			rp->args = nl + 1;
			rp->pq = 0;
			rp++;
		}
	if (dp)
		dp->attrs = dp->global = join->attrs | B(na);
	rp->args = 0;
	return (char *) join;
}

join_close(join)
join_t	*join;
{
	int	rv = 0;
	rel_t	*rp;

	if (!join->pq) {
		for (rp = join->rel; rp->args; rp++)
			if (rp->pq && pq_close(rp->pq) == -1)
				rv = -1;
	} else
		rv = pq_close(join->pq);
	free((char *) join);
	return rv;
}

join_read(join, argv)
join_t	*join;
char	*argv[];
{
	char	*vec[NVEC];
	int	rv, i;
	merge_t	*mp;
	query_t	*qp;
	char	*cp;

	if (join->pq)
		return pq_read(join->pq, argv);
	while (join->list)
		if (! (join->attrs & B(--join->list))) {
			argv[0] = join->attr[join->list];
			argv[1] = 0;
			return 1;
		}
	if (join->rp)
		return pq_read(join->rp->pq, argv);

	for (mp = join->merge; mp->rp && mp->wrote; mp++);
	if (!mp->rp)
		mp--;

	while (mp->rp)
		if (!mp->wrote) {
			cp = mp->base;
			i = 0;
			for (qp = join->query; qp->attr; qp++)
				if (mp->rp->global & B(qp->a))
					if (! (mp->attrs & B(qp->a))) {
						vec[i++] = ++cp;
						cp = strcon(cp,
							join->attr[qp->a],
							"=", qp->val, Z);
					} else
						vec[i++] = qp->attr;
			vec[i] = 0;
			mp->wrote++;
			if (pqowrite(mp->rp, vec) == -1)
				return -1;
		} else if ((rv = pq_read(mp->rp->pq, vec)) > 0) {
			cp = mp->base;
			i = 0;
			for (qp = join->query; qp->attr; qp++)
				if (mp->rp->global & B(qp->a))
					if (mp->attrs & B(qp->a)) {
						qp->val = ++cp;
						cp = strcon(cp, vec[i++], Z);
					} else
						i++;
			(++mp)->base = cp;
		} else if (rv == 0) {
			if (mp == join->merge)
				return 0;
			(mp--)->wrote = 0;
		} else
			return rv;

	for (i = 0; i < join->argc; i++)
		argv[i] = join->query[i].val;
	argv[i] = 0;
	return rv;
}

join_write(join, argv)
join_t	*join;
char	*argv[];
{
	int	attrs = 0, select = 0;
	char	*cp = join->base, *s;
	int	argc, i;
	query_t	*qp;
	merge_t	*mp;
	rel_t	*rp;

	if (join->pq)
		return pq_write(join->pq, argv);

	for (argc = 0, qp = join->query; argv[argc]; argc++, qp++) {
		for (i = 0; s = join->attr[i]; i++)
			if (eqattr(argv[argc], s))
				break;
		if (eqattr(argv[argc], "attribute"))
			join->list = i;
		attrs |= B(i);
		qp->attr = s;
		qp->val = 0;
		qp->a = i;
	}
	for (rp = join->rel; rp->args; rp++)
		if (! (attrs & ~rp->attrs) || rp->na == 1 && attrs & rp->attrs)
			return pqowrite(join->rp = rp, argv);

	for (argc = 0, qp = join->query; argv[argc]; argc++, qp++)
		if ((s = strchr(argv[argc], '=')) || !qp->attr) {
			qp->attr = ++cp;
			cp = strcon(cp, argv[argc], Z);
			if (s)
				select |= B(qp->a);
		}
	for (rp = join->rel; rp->args; rp++)
		if (rp->na > 1 && attrs & rp->global
		 && ! (attrs & B(i = key(rp->attrs)))) {
			attrs |= B(i);
			qp->attr = join->attr[i];
			qp->val = 0;
			qp->a = i;
			qp++;
		}
	join->argc = argc;
	join->rp = 0;
	qp->attr = 0;
	mp = join->merge;
	mp->base = cp;

	while (attrs) {
		for (rp = join->rel; rp->args; rp++)
			if (select & rp->global && attrs & rp->global) {
				mp->attrs = attrs & rp->global;
				select |= mp->attrs;
				attrs &= ~rp->global;
				mp->wrote = 0;
				mp->rp = rp;
				mp++;
				break;
			}
		if (!rp->args)
			select |= attrs;
	}
	mp->rp = 0;
	return 1;
}

static char *
mpqopen(args)
char	*args;
{
	char	*str, *nl, *pq;
	char	*argv[512];
	char	buf[1024];

	for (str = strcpy(buf, args); nl = strchr(str, '\n'); str = ++nl) {
		*nl = 0;
		strvec(str, argv, " \t\n");
		if (pq = pq_open(argv))
			return pq;
	}
	if (str == buf)
		error(0, "join: No modules");
	return 0;
}

static int
pqowrite(rp, argv)
rel_t	*rp;
char	*argv[];
{
	return rp->pq || (rp->pq = mpqopen(rp->args)) ?
		pq_write(rp->pq, argv) : -1;
}

static int
key(a)
int	a;
{
	int i = 0;
	while (! (a & B(i++)));
	return --i;
}
