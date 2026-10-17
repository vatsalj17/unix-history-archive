#include	<stdio.h>
#include	<ctype.h>

#include	"gen.h"
#include	"option.h"
#include	"md.h"

LOCAL	FILEPTR	p_offset	= -1;
LOCAL	FILEPTR	h_offset	= -1;
LOCAL	FILEPTR	f_offset	= -1;

LOCAL	bool	display_header	= TRUE;
LOCAL	bool	display_footer	= TRUE;

LOCAL	int	document_count;
LOCAL	int	header_count;
LOCAL	int	footer_count;

int	page_cnt		= 0;
int	page_no			= 0;
bool	title_page		= FALSE;

read_globals()
	{

	int	version;
	REG	int	i;
	int	length;

	if( (version = get_int(infd)) != MW_ID )
		warning( "Unknown Macwrite version %d", version );

	p_offset = get_int(infd);
	document_count = get_int(infd);
	header_count = get_int(infd);
	footer_count = get_int(infd);

	title_page = get_byte(infd);
	(void)get_byte(infd);			/* display scrap */
	display_footer =  get_byte(infd);
	display_header =  get_byte(infd);
	(void)get_byte(infd);			/* rulers showing */
	(void)get_byte(infd);			/* spare byte (?) */
	(void)get_byte(infd);			/* active document */
	page_no = get_int(infd) - 1;

	/*  get the seek locations of the header and footer info  */
	set( infd, p_offset );
	for( i=0; i<document_count; i++ )
		{
		(void)get_int(infd);	/* paragraph type */
		length = get_int(infd);	/* paragraph length */

		skip( infd, length );
		}
	
	h_offset = pos(infd);


	for( i=0; i<header_count; i++ )
		{
		(void)get_int(infd);	/* paragraph type */
		length = get_int(infd);	/* paragraph length */

		skip( infd, length );
		}
	
	/*	set the bottom margin, based on the footer length */
	f_offset = pos(infd);
	init_footer();

	if( verbose )
		{
		fprintf( stderr, "Macwrite(%d) %d header, %d body, %d footer paragraghs\n", 
		version, header_count, document_count,  footer_count );
		fprintf( stderr, "footers %s headers %s\n", 
			display_footer ? "displayed" : "hidden",
			display_footer ? "displayed" : "hidden" );
		}
	}


read_paragraphs( off, count )
FILEPTR	off;
int	count;
	{
	int	i;

	set(infd, off);

	for( i=0; i<count; i++ )
		paragraph();
	
	}

body()
	{
	read_paragraphs( p_offset, document_count );
	}

header()
	{
	FILEPTR	save_pos;

	if( !display_header )
		return;

	save_pos = pos(infd);
	save_ruler();

	read_paragraphs( h_offset, header_count );

	restore_ruler();
	set( infd, save_pos );
	}


footer()
	{
	FILEPTR	save_pos;

	if( !display_footer )
		return;

	save_pos = pos(infd);
	save_ruler();

	read_paragraphs( f_offset, footer_count );

	restore_ruler();
	set( infd, save_pos );
	}


paragraph()
	{
	int	type;
	int	length;
	int	line_length;
	int	flength;
	FILEPTR	paragraph_end;
	REG	int	i;

	byte	cur_style;
	byte	cur_size;
	short	cur_font;

	Char	buf[200];
	bool	freeline = FALSE;
	Char	* line = buf;

	int	chpos;
	int	end_pos;


	if( (type = get_int(infd)) != RULER 
	&&  type != TEXT
	&&  type != PICTURE )
		fatal( "botch - invalid paragraph type %d in macwrite file",
			type );
	
	length = get_int(infd);
	paragraph_end = pos(infd) + length;

	if( type == TEXT )
		{

		line_length = get_int(infd);
		if( line_length > 200 )
			{
			line = (Char *)malloc( sizeof(Char) * line_length );
			if( line == NULL )
				fatal( "fatal - out of space\n" );
			freeline = TRUE;
			}


		for( i=0; i<line_length; i++ )
			line[i].c_char = get_byte(infd);

		if( line_length % 2 )
			skip(infd,1);

		/*	get font info	*/
		if( (flength = get_int(infd)) % 6 != 0 )
			{
			fatal( "invalid font length (%d) in macwrite file\n",
			flength );
			}

		for( chpos = get_int(infd); flength; chpos = end_pos )
			{
			cur_size = get_byte(infd);
			cur_style = get_byte(infd);
			cur_font = get_int(infd);

			flength -= 6;
			if( flength )
				end_pos = get_int(infd);
			else
				end_pos = line_length;

			
			for( i=chpos; i<end_pos; i++ )
				{
				line[i].c_size = cur_size;
				line[i].c_style = cur_style;
				line[i].c_font = cur_font;
				}
			}
		
		/*
		 *	mystery bytes 
		 */
		set( infd, paragraph_end );

		out_line( line_length, line );

		if( freeline )
			free( line );
		}
	else
	if( type == PICTURE )
		{

		/*	page breaks are pictures	*/
		if( (length = get_int(infd)) == 0 )
			page();
		else
			warning( "pictures not yet supported" );
		set( infd, paragraph_end );

		}
	else
	if( type == RULER )
		{
		PIX	lmargin;
		PIX	rmargin;
		PIX	indent;
		PIX	tab;
		int	tablen;
		int	spacing;
		int	justify;

		lmargin = get_int(infd);
		rmargin = get_int(infd);

		justify = get_byte(infd);
		
		tablen = get_byte(infd);
		get_byte(infd);		/* ??? mystery byte */

		spacing = get_byte(infd);
		indent = get_int(infd);

		set_ruler( lmargin, rmargin, indent, spacing, justify );

		/*
		 *	pick up the tab stops
		 */
		clr_tabs();

		for( i=0; i<tablen; i++ )
			{
			set_tab( get_int(infd) );
			}
		
		for( i=0; i < 10-flength; i++ )
			get_int(infd);
		
		/*
		 *	more mystery bytes
		 */
		get_int(infd);
		get_int(infd);

		set( infd, paragraph_end );

		}

	}


get_int(fd)
FILE	* fd;
	{
	int	ch;

	ch = get_byte(fd);
	ch = ch << 8 | get_byte(fd);

	return( ch );
	}

long
get_long(fd)
FILE	* fd;
	{
	long	val;
	int	i;

	for( val=0, i=0; i<4; i++ )
		{
		val <<= 8;
		val |= get_byte(fd);
		}
	
	return( val );
	}

int
get_byte(fd)
FILE	* fd;
	{
	int	ch;

	if( (ch = getc(fd)) == EOF )
		fatal( "unexpected EOF" );

	return( ch & 0xff );
	}


char	* 
Style( style_no )
byte	style_no;
	{

	style_no &= (ST_PLAIN|ST_BOLD|ST_ITALIC);

	switch( style_no )
		{
		case ST_PLAIN:
			return( "r" );

		case ST_BOLD:
			return( "b" );

		case ST_ITALIC:
			return( "ti" );

		default:
			fprintf( stderr, "unknown style %d\n", style_no );
			break;
		}
	return( "??" );

	}
