struct map
{
	short	m_size;
	unsigned short m_addr;
};

struct map coremap[MAPSIZE];	/* space for core allocation */
struct map swapmap[MAPSIZE];	/* space for swap allocation */
