/*
 *	structure to describe contents of mount table
 */

#define	NMOUNT	16
#define	NAMSIZ	32

struct mtab
{
	char	m_file[NAMSIZ];
	char	m_spec[16];
	long	m_time;
	int	m_prot;
	char	m_padding[10];
}
	mtab[NMOUNT];

char	mtabf[]	"/etc/mtab";
