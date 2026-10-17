/*
 *	Sydney C Compiler.
 *
 *	Copyright 1984, Bruce Ellis.
 *
 *	Unauthorised possesion, sale or use prohibited.
 */
/*
 *  The null machine (cyntax checker or compiler debugger).
 */
#define	BITS_PER_BYTE	8
#define	MAXINTBITS	0xFFFFFFFFL
#define	MAXOF_INT	0x7FFFFFFFL
#define	MINOF_INT	0x80000000L

#define	BITS_PER_CHAR	(1*BITS_PER_BYTE)
#define	BITS_PER_SHORT	(2*BITS_PER_BYTE)
#define	BITS_PER_INT	(4*BITS_PER_BYTE)
#define	BITS_PER_LONG	(4*BITS_PER_BYTE)
#define	BITS_PER_PTR	(4*BITS_PER_BYTE)
#define	BITS_PER_FLOAT	(4*BITS_PER_BYTE)
#define	BITS_PER_DOUBLE	(8*BITS_PER_BYTE)

#define	CHAR_SIGN	0x80
#define	CHAR_SXT(i)	(-((~(i) + 1) & 0x7F))

