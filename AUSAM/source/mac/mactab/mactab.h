#define	NE1	8
#define	NS1	20
#define	NS3	4

#define	NLIT	32
#define	NSYM	32
#define	NFMT	32

#define	NTREE	256
#define	NTABLE	400


struct	node	{
	int	n_sym;
	int	n_mem;
	char	n_mem4[4];
	struct	node	*n_alt;
	struct	node	*n_next;
	};
