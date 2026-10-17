/*
 * Query optimization module
 */

#include <malloc.h>
#include <string.h>

typedef struct {
	char	*tag;
	short	count;
	short	copy[512];
} opt_t;

extern char	*pq_open();

char *
opt_open(argv)
char	*argv[];
{
	opt_t	*opt = (opt_t *) malloc(sizeof (opt_t));

	if ((opt->tag = pq_open(argv)) == 0) {
		free((char *) opt);
		return 0;
	}
	opt->count = -1;
	return (char *) opt;
}

opt_close(opt)
opt_t	*opt;
{
	int rv = pq_close(opt->tag);
	free((char *) opt);
	return rv;
}

opt_read(opt, argv)
register opt_t	*opt;
register char	*argv[];
{
	register int	rv, i;

	if ((rv = pq_read(opt->tag, argv)) > 0)
		for (i = opt->count; i >= 0; i--)
			argv[i] = argv[opt->copy[i]];
	return rv;
}

opt_write(opt, argv)
register opt_t	*opt;
register char	*argv[];
{
	register int	i, j, k = 0;
	char	*vec[512];

	for (i = 0; argv[i]; i++) {
		for (j = 0; j < k; j++)
			if (eqattr(argv[i], vec[j])) {
				if (!strchr(vec[j], '='))
					vec[j] = argv[i];
				break;
			}
		if (j == k)
			vec[k++] = argv[i];
		opt->copy[i] = j;
	}
	opt->copy[i] = k;
	opt->count = i;
	vec[k] = 0;
	return pq_write(opt->tag, vec);
}
