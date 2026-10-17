#define	MAX_FONTS	64


/*	macintosh predefined font numbers */
#define	F_SYSTEM (0)
#define	F_APPL	(1)
#define	F_NEWYORK (2)
#define	F_GENEVA (3)
#define	F_MONACO (4)
#define	F_VENICE (5)
#define	F_LONDON (6)
#define	F_ATHENS (7)
#define	F_SANFRAN (8)
#define	F_TORONTO (9)


typedef	struct
	{
	byte	ch_windex;
	byte	ch_hindex;
	byte	ch_itindex;
	byte	ch_remainder;
	} CH_INFO;
	
typedef	struct
	{
	int	f_number;		/* mac font number */
	byte	f_size;
	byte	f_style;		/* bold, italic, etc. */
	bool	f_active;		/* does the driver know about me? */
	char	* f_name;		/* font name (cmr, etc.) */
	long	f_csum;	
	CH_INFO	* f_info;		/* tfm info */
	SP	* f_width;
	SP	* f_height;
	SP	f_maxheight;
	SP	* f_depth;
	SP	f_dsize;
	SP	f_at_size;
	SP	f_slant;
	SP	f_stretch;
	SP	f_shrink;
	SP	f_accent;
	SP	f_ss;
	SP	f_ws;
	} FONT;

FONT	fonts[MAX_FONTS];

SP	ch_wid();
SP	font_ws();
SP	font_depth();
SP	font_height();
