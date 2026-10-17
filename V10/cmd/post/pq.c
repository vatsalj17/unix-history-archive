/*
 * Switch module
 */

#include <malloc.h>
#include "switch.h"

#define	DEFMOD	"opt", "join"

typedef struct {
	modsw_t	*mod;
	char	*tag;
} pq_t;

char *
pq_open(argv)
char	*argv[];
{
	register pq_t	*pq = (pq_t *) malloc(sizeof (pq_t));
	static char	*defv[] = { DEFMOD, 0 };

	if (!argv || !argv[0])
		argv = defv;
	for (pq->mod = modsw; pq->mod->name; pq->mod++)
		if (!strcmp(argv[0], pq->mod->name)) {
			if (pq->tag = (*pq->mod->open)(&argv[1]))
				return (char *) pq;
			break;
		}
	if (!pq->mod->name)
		error(0, "pq %s: Not configured", argv[0]);
	free((char *) pq);
	return 0;
}

pq_close(pq)
pq_t	*pq;
{
	int rv = pq ? (*pq->mod->close)(pq->tag)
		    : error(0, "pq: close: Not opened");
	if (pq)
		free((char *) pq);
	return rv;
}

pq_read(pq, arg, n)
pq_t	*pq;
char	*arg;
{
	return pq ? (*pq->mod->read)(pq->tag, arg, n)
		  : error(0, "pq: read: Not opened");
}

pq_write(pq, arg, n)
pq_t	*pq;
char	*arg;
{
	return pq ? (*pq->mod->write)(pq->tag, arg, n)
		  : error(0, "pq: write: Not opened");
}
