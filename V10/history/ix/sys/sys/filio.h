/*
 * general file and stream ioctls
 */

/*
 * for FIOINSLD
 */
struct	insld {
	short	ld;
	short	level;
};

/*
 * for passing files across streams
 */
struct	passfd {
	int	fd;
	short	uid;
	short	gid;
	short	nice;
	char	logname[8];
	char	cap;
};

/* for FIOGSR, FIOSSRC */

#define SSRCSIZ 32

/*
 * file ioctls
 * (see pex.h for more)
 */
#define	FIOCLEX		(('f'<<8)|1)
#define	FIONCLEX	(('f'<<8)|2)
#define	FIOPUSHLD	(('f'<<8)|3)
#define	FIOPOPLD	(('f'<<8)|4)
#define	FIOLOOKLD	(('f'<<8)|5)
#define FIOINSLD	(('f'<<8)|6)
#define	FIOSNDFD	(('f'<<8)|7)
#define	FIORCVFD	(('f'<<8)|8)
#define	FIOACCEPT	(('f'<<8)|9)
#define	FIOREJECT	(('f'<<8)|10)
#define	FIOGSRC		(('f'<<8)|106)
#define	FIOSSRC		(('f'<<8)|107)
#define	FIONREAD	(('f'<<8)|127)
