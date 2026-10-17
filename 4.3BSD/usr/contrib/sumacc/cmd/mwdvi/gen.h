

#define	FALSE	(0)
#define	TRUE	(!FALSE)

#define	L_SET	0
#define	L_CURR	1
#define	L_EOF	2

#define	REG	register
#define	LOCAL	static


#define	Debug		if( debflg )
#define	skip(fd,count)	(fseek(fd, (FILEPTR)(count), L_CURR))
#define	set(fd,pos)	(fseek(fd, (FILEPTR)(pos), L_SET))
#define	pos(fd)		(ftell(fd))
#define	align(fd)	if( pos(fd) % 2 ) Skip( 1 )

#define	Skip(count)	(skip(stdin,count))
#define	Set(pos)	set(stdin,pos)
#define	Get_int()	get_int(stdin)
#define	Get_byte()	get_byte(stdin)
#define	Pos()		(pos(stdin))
#define	Align()		align(stdin)

#define	max(a,b)	( (a) > (b) ? (a) : (b) )


typedef	unsigned char	byte;
typedef	byte	bool;

typedef	long	RSU, FIX, SP;		/* tex units */
typedef	long	FILEPTR;

typedef	struct
	{
	short	c_char;
	short	c_font;
	byte	c_size;
	byte	c_style;
	} Char;


extern	long	get_long();
extern	int	get_int();
extern	int	get_byte();

extern	char	* Style();

extern	bool	debflg;
