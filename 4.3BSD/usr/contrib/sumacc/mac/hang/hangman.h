
# define	TRUE	1
# define	FALSE	0
# define	NIL		0
# define	bool	char
# define	BUFSIZ	80
# define	NULL	0
# define	ERROR	-1

# define	MAXTRY	100  /* how many tries to get good word from dictionary */

typedef 	int 	off_t ;

# define	FILE	int
# define	QUITCHAR	0x09   /* tab char */
#define		isalpha(c)	(((c>='A')&&(c<='Z'))||((c>='a')&&(c<='z'))) 
#define		isupper(c)	((c>='A')&&(c<='Z'))
#define		islower(c)	((c>='a')&&(c<='z'))
#define 	tolower(c)	((c)-'A'+'a')

# define	STARTH 85
# define	STARTV 25

# define	MINLEN	6
# define	MAXERRS	7
# define	DICT	"words"

# define	LINEHEIGHT	16
# define	MESGH	15
# define	MESGV	170
# define	PROMPTH	MESGH
# define	PROMPTV	MESGV + LINEHEIGHT
# define	KNOWNH	MESGH
# define	KNOWNV	PROMPTV + LINEHEIGHT

# define	GUESSH	210
# define	GUESSV	30
# define	NUMBERH	GUESSH
# define	NUMBERV	GUESSV + 3 * LINEHEIGHT
# define	AVGH	GUESSH
# define	AVGV	NUMBERV + LINEHEIGHT


extern bool	Guessed[];

extern char	Word[], Known[], *Noose_pict[];

extern int	Errors, Wordnum;

extern float	Average;

extern FILE	*Dict;

extern off_t	Dict_size;

int	die();

off_t	abs();
