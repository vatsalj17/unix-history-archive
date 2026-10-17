#include	<stdio.h>
#include	<ctype.h>

#include	"site.h"
#include	"gen.h"
#include	"option.h"
#include	"md.h"
#include	"dvi.h"
#include	"fonts.h"

LOCAL	char	* get_tok();
LOCAL	char	* mac_font();

/*
 *	translate apple font numbers into something more reasonable
 */
fontno( lptr )
Char	* lptr;
	{
	REG	int	i;
	REG	int	j;
	REG	FONT	* fnt;

	char	buf[200];
	char	obuf[200];
	FILE	* fd;
	int	bc, ec;
	int	hlength;
	int	wlength;
	int	dlength;
	int	extra_length;
	int	hdlength;

	int	at_size;
	double	ratio;


	for( i=0; i<MAX_FONTS; i++ )
		{
		if( fonts[i].f_number == -1 )
			break;
		else
		if( fonts[i].f_number == lptr->c_font
		&&  fonts[i].f_size == lptr->c_size
		&&  ( (fonts[i].f_style & ~(ST_UL|ST_SHADOW)) == 
		      (lptr->c_style & ~(ST_UL|ST_SHADOW)) )  )
			return( i );
		}
	
	if( i == MAX_FONTS )
		fatal( "too many fonts" );

	fnt = &fonts[i];
	

	/*
	 *	no such font - load one up
	 */

	at_size = lptr->c_size;

	/*	default font is a scaled 10pt font	*/
	sprintf( buf, "cm%s10", Style(lptr->c_style) );
	substitute_font( buf, lptr, &at_size );

	if( verbose )
		fprintf( stderr, "using font %s at %dpt for MAC font %s\n", 
			buf, at_size, mac_font(lptr) );

	strcat( buf, ".tfm" );

	fnt->f_number = lptr->c_font;
	fnt->f_size = lptr->c_size;
	fnt->f_style = lptr->c_style;

	sprintf( obuf, "%s/%s", TFM_PATH, buf );

	buf[strlen(buf)-4] = '\0';
	fnt->f_name = (char *)(strcpy( (char *)malloc(strlen(buf)+1),buf));


	/*
	 *	read in the tfm file
	 */
	if( (fd = fopen(obuf, "r")) == NULL )
		fatal( "couldn't open font tfm file %s", obuf );
	
	get_int(fd);			/* lf */
	hdlength = get_int(fd);		/* header data */
	bc = get_int(fd);		/* first character */
	ec = get_int(fd);		/* last character */
	wlength = get_int(fd);		/* lengths of sections */
	hlength = get_int(fd);
	dlength = get_int(fd);

	extra_length = get_int(fd) + get_int(fd) +
		get_int(fd) + get_int(fd);	/* length of the rest */

	fnt->f_info = (CH_INFO *)malloc( (ec-bc+1) * sizeof(CH_INFO) );
	fnt->f_width = (long *)malloc( wlength * 4 );
	fnt->f_height = (long *)malloc( hlength * 4 );
	fnt->f_depth = (long *)malloc( dlength * 4 );

	set( fd, 6 * sizeof(long) );

	fnt->f_csum = get_long(fd);
	fnt->f_dsize = fix_to_sp(get_long(fd));
	fnt->f_at_size = pt_to_sp(at_size);

	set( fd, (6 + hdlength) * 4 );

	for( j=0; j<(ec-bc+1); j++ )
		{
		fnt->f_info[j].ch_windex = get_byte(fd);
		fnt->f_info[j].ch_hindex = get_byte(fd);
		fnt->f_info[j].ch_itindex = get_byte(fd);
		fnt->f_info[j].ch_remainder = get_byte(fd);
		}
	
	for( j=0; j<wlength; j++ )
		fnt->f_width[j] = fix_to_sp(get_long(fd) * at_size);

	for( j=0; j<hlength; j++ )
		fnt->f_height[j] = fix_to_sp(get_long(fd) * at_size);

	for( j=0; j<dlength; j++ )
		fnt->f_depth[j] = fix_to_sp(get_long(fd) * at_size);
	
	/*
	 *	(fix this right) - grab a big character to set the pace
	 */
	fnt->f_maxheight = 
		fnt->f_height[(fnt->f_info['['].ch_hindex>>4) & 0xf];

	skip( fd, extra_length * 4 );

	/*	parameter words	*/
	(void)get_long(fd);					/* slant */
	fnt->f_ws = fix_to_sp(get_long(fd) * at_size);		/* space */
	fnt->f_stretch = fix_to_sp(get_long(fd) * at_size);	/* stretch */
	fnt->f_shrink = fix_to_sp(get_long(fd) * at_size);	/* shrink */
	fnt->f_accent = fix_to_sp(get_long(fd) * at_size);	/* accent */
	(void)get_long(fd);					/* quad */
	fnt->f_ss = fnt->f_ws + fix_to_sp(get_long(fd) * at_size); /*sentence*/
	
	fclose(fd);

	return( i );

	}

/*
 *	perform font substitutions from a file with the format:
 *
 *	font pointsize style  -> newfont atsize
 */

LOCAL	char	*
get_tok( ptr )
REG	char	* * ptr;
	{

	char	* pstart = *ptr;

	while( **ptr && !isspace( **ptr ) )
		(*ptr)++;
	
	if( **ptr )
		*((*ptr)++) = '\0';
	
	return( pstart );

	}


LOCAL
skip_blank( ptr )
REG	char	* * ptr;
	{

	while( **ptr && isspace( **ptr ) )
		(*ptr)++;
	
	return( **ptr != '\0' );

	}



substitute_font( buf, cptr, at_size )
char	* buf;
Char	* cptr;
int	* at_size;
	{
	FILE	* fd;
	char	tbuf[200];
	char	* font;
	char	* ptr;
	REG	char	* tok;
	char	*	rc;
	int	fno;
	int	style;
	int	pt;
	int	st;
	int	line_no;

	if( !font_sub_file )
		return;

	if( (fd = fopen( font_sub_file, "r" )) == NULL )
		{
		warning( "couldn't open substitution file %s", font_sub_file );
		return;
		}
	
	/*	substitute only the rational fonts	*/
	style = cptr->c_style & (ST_PLAIN|ST_BOLD|ST_ITALIC);
	
	for( line_no=1; (rc = fgets( tbuf, 200, fd )) != NULL; line_no++ )
		{

		if( *tbuf == '#' )
			continue;

		st = pt = -1;
		ptr = tbuf;


		/*
		 *	font name or number
		 */
		if( !skip_blank( &ptr ) )
			break;
		
		font = get_tok( &ptr );
		if( isdigit( *font ) )
			fno = atoi( font );
		else
			{
			if( (fno = f_ord( font )) < 0 )
				break;
			}

		/*
		 *	style or pt size or ->
		 */
		if( !skip_blank( &ptr ) )
			break;

		tok =  get_tok( &ptr );

		/*	could have default style, pt, or both at this point */
		if( strcmp( "->", tok ) != 0 && !isdigit( *tok ) )
			{
			if( (st = st_ord( tok )) < 0 )
				break;
			

			if( !skip_blank( &ptr ) )
				break;
			
			tok =  get_tok( &ptr );
			}
		
		if( isdigit( *tok ) )
			{
			pt = atoi( tok );


			if( !skip_blank( &ptr ) )
				break;

			tok = get_tok( &ptr );
			}

		/*	gots to have that arrow	*/
		if( strcmp( "->", tok ) != 0 )
			break;
		

		if( fno == cptr->c_font
		&&  (st == -1 || st == style)
		&&  (pt == -1 || pt == cptr->c_size) )
			{
			/*	match!		*/

			if( !skip_blank( &ptr ) )
				break;
			
			strcpy( buf, get_tok( &ptr ) );

			if( skip_blank( &ptr ) )
				*at_size = atoi( get_tok( &ptr ) );

			return;

			}
		

		}
	
	if( rc != NULL )
		warning( "syntax error in font substitution file at line %d",
			line_no );
	
	fclose( fd );

	}

/*	macintosh predefined fonts */
LOCAL	
char	* macfonts[] =
	{
	"system",
	"application",
	"new_york",
	"geneva",
	"monaco",
	"venice",
	"london",
	"athens",
	"san_fran",
	"toronto",
	};

f_ord( font )
char	* font;
	{

	char	* * fptr;

	for( 
	     fptr = macfonts;
	     fptr < &macfonts[sizeof(macfonts)/sizeof(char *)];
	     fptr++
	   )
		if( strcmp( font, *fptr ) == 0 )
			return( fptr - macfonts );
	
	return( -1 );
	
	}

st_ord( style )
char	* style;
	{

	if( strcmp( "plain", style ) == 0 )
		return( ST_PLAIN );
	
	else
	if( strcmp( "bold", style ) == 0 )
		return( ST_BOLD );
	else
	if( strcmp( "italics", style ) == 0 )
		return( ST_ITALIC );
	else
	if( strcmp( "bold_italics", style ) == 0 )
		return( ST_BOLD | ST_ITALIC );
	
	return( -1 );

	}


/*	print out the name of the macintosh font indicated	*/
LOCAL	char *
mac_font( cptr )
Char	* cptr;
	{
	LOCAL	char	buf[40];

	if( cptr->c_font >= 0 
	&&  cptr->c_font < (sizeof(macfonts)/sizeof(char *)) )
		strcpy( buf, macfonts[ cptr->c_font ] );
	else
		sprintf( buf, "#%d", cptr->c_font );
	
	if( cptr->c_style & ST_BOLD )
		strcat( buf, " bold" );
	
	if( cptr->c_style & ST_ITALIC )
		strcat( buf, " italic" );
	
	return( buf );
	
	}


		

/*
 *	put a definition of the indicated font into the dvi file
 */
	
def_font( fno )
int	fno;
	{
	REG	int	i;

	if( fno < 0 || fno > MAX_FONTS ||
	    fonts[fno].f_number < 0 )
		fatal( "unknown font: %d\n", fno );


	outcmd( DFONT1 );
	out1( fno );
	out4( fonts[fno].f_csum );		/* checksum */
	out4( fonts[fno].f_at_size  );		/* num */
	out4( fonts[fno].f_dsize );		/* denom */
	out1( 0 );				/* use the system fonts */
	out1( strlen( fonts[fno].f_name ) );
	for( i=0; i<strlen(fonts[fno].f_name ); i++ )
		out1( fonts[fno].f_name[i] );

	fonts[fno].f_active = TRUE;
	
	}


SP
font_height( fontno )
int	fontno;
	{

	return( fonts[fontno].f_maxheight );

	}

SP
font_ws( fontno )
int	fontno;
	{

	return( fonts[fontno].f_ws );

	}

SP
font_ss( fontno )
	{

	return( fonts[fontno].f_ss );

	}


SP
ch_wid( cptr )
Char	* cptr;
	{
	REG	FONT	* fno;

	fno = &fonts[fontno(cptr)];

	return(fno->f_width [ (fno->f_info[cptr->c_char]).ch_windex ]);

	}

SP
ch_height( cptr )
Char	* cptr;
	{
	REG	FONT	* fno;

	fno = &fonts[fontno(cptr)];

	return( fno->f_height[(fno->f_info[cptr->c_char].ch_hindex>>4) & 0xf] );

	}

SP
ch_depth( cptr )
Char	* cptr;
	{
	REG	FONT	* fno;

	fno = &fonts[fontno(cptr)];

	return( fno->f_depth[fno->f_info[cptr->c_char].ch_hindex & 0xf] );

	}
