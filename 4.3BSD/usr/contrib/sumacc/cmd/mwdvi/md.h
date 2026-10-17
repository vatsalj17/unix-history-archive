
#define	MW_ID	3		/* macwrite version */

#define	RULER	0		/* paragraph type */
#define	TEXT	1
#define	PICTURE	2

#define	ST_PLAIN (0)		/* style */
#define	ST_BOLD	(0x1)
#define	ST_ITALIC (0x2)
#define	ST_UL (0x4)
#define	ST_SHADOW (0x8)

#define	JST_LEFT (0)		/* line justification	*/
#define	JST_CENTER (1)
#define	JST_RIGHT (2)
#define	JST_ADJ (3)

#define	S_SINGLE (0)		/* line spacing */
#define	S_ONEANDHALF (1)
#define	S_DOUBLE (2)


typedef	long	PIX;


extern	int	page_no;	/* current page number */
extern	int	page_cnt;	/* how many pages we got? */
extern	bool	title_page;	/* first page is title page */
