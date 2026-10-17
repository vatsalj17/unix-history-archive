#include	<stdio.h>
#include	<ctype.h>

#include	"gen.h"
#include	"option.h"
#include	"md.h"
#include	"dvi.h"
#include	"fonts.h"


LOCAL	SP	indent		= min_to_sp(   500 );
LOCAL	SP	lmargin_h	= min_to_sp(   500 );
LOCAL	SP	lmargin_v	= min_to_sp(  1000 );
LOCAL	SP	rmargin_v	= min_to_sp( 10500 );
LOCAL	SP	rmargin_h	= min_to_sp(  6500 );
LOCAL	SP	tabs[TABMAX]	= { 0, };
LOCAL	int	tabcnt		= 0;
LOCAL	double	spacing	= 1.0;
LOCAL	int	adjust = JST_LEFT;


LOCAL	int	cur_font;
LOCAL	int	pushmax		= 0;
LOCAL	POS	stack[20];
LOCAL	POS	* cur;

LOCAL	FILEPTR	last_page = -1;


LOCAL	bool	silent = FALSE;


init()
	{
	int	i;

	for( i=0; i<MAX_FONTS; i++ )
		{
		fonts[i].f_active = FALSE;
		fonts[i].f_number = -1;
		}
	

	preamble();

	page_no = 0;
	page();

	}

finish()
	{
	outcmd( EOP );
	postamble();
	}

preamble()
	{
	outcmd( TEX_ID );		/* tek version 2 */
	out4( scale_num );		/* num */
	out4( scale_denom );		/* denom  */
	out4( 1000 );			/* mag */
	out1( 0 );
	}

postamble()
	{
	long	postptr;
	int	i;


	postptr = ftell( outfd );
	outcmd( POST );
	out4( last_page );
	out4( scale_num );		/* num */
	out4( scale_denom );		/* denom  */
	out4( 1000 );			/* mag */
	out4( min_to_sp( 11000 ) );	/* max height (fix this) */
	out4( min_to_sp( 8500 ) );	/* max width (fix this) */
	out2( pushmax );		/* max stack depth  */
	out2( page_no );

	/*	font definitions	*/
	for( i=0; i<MAX_FONTS; i++ )
		{
		if( fonts[i].f_number == -1 )
			break;
		else
			def_font( i );
		}
	
	outcmd( POSTPOST );
	out4( postptr );
	out1( TEX_ID );

	/* output a signature	*/
	for( i=0; i<4; i++ )
		out1( 223 );
	
	postptr = ftell(outfd);
	
	for( i=0; i< (postptr % 4); i++ )
		out1( 223 );

	}



/*
 *	do a page eject
 */
LOCAL	in_page = FALSE;
LOCAL	in_footer = FALSE;

page()
	{

	SP	bottom_margin;

	if( in_page )
		{

		if( in_footer )
			{
			warning( "footer extends off the bottom of the page" );
			return;
			}

		bottom_margin = rmargin_v;

		cur->p_v = rmargin_v;
		rmargin_v = min_to_sp( 11500 );
		in_footer = TRUE;

		footer();

		in_footer = FALSE;
		rmargin_v = bottom_margin;

		outcmd( EOP );

		}

	bop( ++page_no );
	in_page = TRUE;

	/*	potentially, don't print header on title page  */
	if( page_cnt++ || !title_page )
		header();

	}



bop( page )
int	page;
	{
	long	lastp;
	int	i;

	lastp = ftell(outfd);

	outcmd( BOP );
	out4( page );
	for( i=0; i<9; i++ )
		out4( 0 );
	out4( last_page );

	last_page = lastp;

	cur = stack;
	cur->p_v = cur->p_h = cur->p_w = 
	cur->p_x = cur->p_y = cur->p_z = 0;
	cur_font = -1;

	/*	start us out someplace reasonable	*/
	out_pos( 0L, lmargin_v );

	}


push()
	{
	if( cur - stack >= 19 )
		fatal( "botch: stack overflow" );
	
	outcmd( PUSH );

	*(cur+1) = *cur;
	++cur;

	pushmax = max( pushmax, cur - stack );
	}

pop()
	{
	if( cur <= stack )
		fatal( "botch: stack underflow" );
	
	outcmd( POP );

	--cur;
	}



/*
 *	format another line, dvi style
 */

out_line( len, line )
int	len;
Char	* line;
	{
	WORD	words[MAX_WORDS];

	int	wordpos;
	int	cpos;

	int	i;
	int	j;

	int	cur_font;
	int	this_font;

	SP	line_length;
	SP	line_height;
	SP	word_length;
	SP	word_space;
	SP	ss_space;
	SP	margin;

	/*
	 *	initial indent
	 */
	margin = indent;

	/*
	 *	break this paragraph into words
	 */
	for( cpos=0; cpos < len; ) {

	/*
	 *	skip leading spaces
	 */
	for( ; cpos < len; cpos++ )
		if( line[cpos].c_char != ' ' )
			break;

	cur_font = this_font = fontno( &line[cpos] );
	word_space = font_ws( cur_font );
	ss_space = font_ss( cur_font );
	line_height = font_height( cur_font );

	wordpos = 0;
	line_length = 0;

	/* leave yourself an initial null space */
	words[wordpos].wd_length = 0;
	words[wordpos++].wd_flags = 0;

	while( cpos < len && line_length < (rmargin_h - margin) )
		{

		if( wordpos >= MAX_WORDS )
			fatal( "(botch) too many words in this paragraph" );

		if( !isspace( line[cpos].c_char ) )
			{
			/*
			 *	non space characters
			 */
			words[wordpos].wd_flags = WD_WORD;
			words[wordpos].wd_pos = cpos;

			
			word_length = 0;
			while( cpos < len 
			&&     !isspace( line[cpos].c_char ) )
				{
				word_length += ch_wid( &line[cpos] );

				this_font = fontno( &line[cpos] );
				line_height = max( line_height,
					font_height( this_font ) );

				cpos++;
				}
			
			/*
			 *	this_font is guaranteed to be set
			 */
			if( this_font != cur_font )
				{
				cur_font = this_font;
				word_space = font_ws( cur_font );
				ss_space = font_ss( cur_font );
				}
			
			words[wordpos].wd_length = word_length;
			words[wordpos++].wd_end = cpos;

			Debug {
			fprintf( stderr, "line_out: -" );
			for( i=words[wordpos-1].wd_pos; i<cpos; i++ )
				fprintf( stderr, "%c", line[i].c_char );
			fprintf( stderr, "- (%2.2f)\n", sp_to_in(word_length),
				sp_to_in(line_length) );
			}

			/*
			 *	will this word overflow the line? 
			 */
			if(line_length + word_length > (rmargin_h - margin))
				{
				cpos = words[--wordpos].wd_pos;
				break;
				}
			}
		else
			{

			SP	space;

			/*
			 *	process the word gap
			 */
			words[wordpos].wd_flags = 0;
			words[wordpos].wd_tabcnt = 0;
			word_length = words[wordpos].wd_length = 0;


			/* 
			 *	is this an inter-word or inter-sentence gap?
			 */
			if( wordpos > 0 
			&&  (words[wordpos-1].wd_flags & WD_WORD)
			&&  ispunct(line[words[wordpos-1].wd_end-1].c_char ) )
				space = ss_space;
			else
				space = word_space;
				

			/* 
			 *	eat up those spaces.  count those tabs
			 */
			while( cpos < len 
			&&     isspace( line[cpos].c_char ) )
				{
				if( line[cpos].c_char == '\t' )
					{
					words[wordpos].wd_flags |= WD_TAB;
					words[wordpos].wd_length = word_space;
					words[wordpos].wd_tabcnt++;
					}
				else
				if( line[cpos].c_char == ' ')
					{
					word_length = 
					words[wordpos].wd_length = space;
					}
				
				if( line[cpos].c_style & ST_UL )
					words[wordpos].wd_flags |= WD_UL;

				cpos++;
				}
				
			wordpos++;
				
			}
		
		line_length += word_length;

		}

	/*

	 *	print out the words in this line
	 */


	/* 	move to the next line */

	down( (SP)((line_height * 1.4) * spacing) );


	push();		/* save the margin coordinates */

	if( cur->p_v > rmargin_v )
		{
		page();	/* you get an implicit pop when you page */
		push();
		}
	right( margin );

	
	/*
	 *	remove trailing white space
	 */
	while( wordpos > 0 )
		{
		if( words[wordpos-1].wd_flags & WD_WORD )
			break;
		else
			wordpos--;
		}

	/*
	 *	center, justify, or whatever
	 */
	justify( words, &wordpos, &cpos, margin, (cpos >= len) );


	/*
	 *	print that suckah
	 */
	for( i=0; i < wordpos; i++ )
		{
		if( words[i].wd_flags & WD_WORD )
			{
			for( j=words[i].wd_pos; j < words[i].wd_end; j++)
				set_ch( &line[j] );
			}
		else
			right( words[i].wd_length );
		}
	
	Debug {
	fprintf( stderr, "line %2.2f, %2.2f\n", sp_to_in(cur->p_h ),
		sp_to_in(cur->p_v) );
	}

	pop();		/* restore the margin coordinates */


	/*	do any underlining that might be appropriate	*/
	underline( words, wordpos, line, margin );
	

	margin = lmargin_h;	/* undo the indent */

	}

	}


/*
 *	justify the line.  add some tabs
 */
justify( words, word_count, cur_pos, margin, last_line )
WORD	* words;
int	* word_count;
int	* cur_pos;
SP	margin;
int	last_line;
	{

	SP	spacelen;
	SP	extra_space;
	SP	line_len;
	SP	line_wid;
	REG	WORD	* cw;
	REG	double	ratio;
	REG	int	i;
	int	lasttab;
	int	wdcnt;

	wdcnt	= *word_count;

	/*
	 *	take care of tabs
	 */
	lasttab = 0;

	if( adjust == JST_LEFT || adjust == JST_ADJ )
		{
		REG	SP cpos = margin;
		int	tb;

		/*  modify tab width to justify next word */
		for( cw=words; cw < &words[wdcnt]; cw++ )
			{
			if( cw->wd_flags & WD_TAB )
				{
				lasttab = (cw-words) + 1;

				for( tb=0; tb < cw->wd_tabcnt; tb++ )
					{

					for( i=0; i<tabcnt; i++ )
						if( cpos < tabs[i] )
							break;

					if( i < tabcnt )
						cw->wd_length = tabs[i] - cpos;

					}
				
				}
			
			/*  is the line now too large to fit? */
			if( cpos + cw->wd_length > rmargin_h )
				break;
				
			cpos += cw->wd_length;

			}
		
		/*	shorten this line.  tabs expanded it too much */
		if( cw != &words[wdcnt] )
			{
			*cur_pos = cw->wd_pos;
			wdcnt = *word_count = cw - words;
			}
		
		/*	don't justify words before a tab stop	*/
		for( cw=words; cw < &words[lasttab]; cw++ )
			margin += cw->wd_length;

		}
	
	/* figure out the current length of this line */
	for  ( line_len = 0, cw = &words[lasttab]; cw < &words[wdcnt]; cw++ )
		line_len += cw->wd_length;
	
	line_wid = rmargin_h - margin;


	switch( adjust )
		{
		case JST_LEFT:
			break;

		case JST_ADJ:
			
			if( last_line || line_len <= 0 )
				break;

			spacelen = 0;

			for( cw = &words[lasttab]; cw < &words[wdcnt]; cw++ )
				{
				if( !(cw->wd_flags & WD_WORD) )
					spacelen += cw->wd_length;
				}
				
			extra_space = (line_wid-line_len) + spacelen;
			ratio = ((double)extra_space) / spacelen;


			for( cw = &words[lasttab]; cw < &words[wdcnt]; cw++ )
				if( !(cw->wd_flags & WD_WORD) )
					cw->wd_length = cw->wd_length * ratio; 
			break;


		case JST_RIGHT:
			words[0].wd_length = line_wid - line_len;
			break;

		case JST_CENTER:
			words[0].wd_length = (line_wid - line_len) / 2;
			break;
		}
	}


underline( words, length, line, margin )
WORD	* words;
int	length;
Char	* line;
SP	margin;
	{

	SP	cpos;
	SP	ul_start;
	REG	int	chpos;
	REG	WORD	* wd;

	bool	ul	= FALSE;

	if( silent )
		return;

	for( cpos = 0, wd = words; wd < &words[length]; wd++  )
		{

		if( wd->wd_flags & WD_WORD )
			{

			for( chpos = wd->wd_pos; chpos < wd->wd_end; chpos++ )
				{
				if( line[chpos].c_style & ST_UL && !ul )
					{
					ul_start = cpos;
					ul = TRUE;
					}
				else
				if( !(line[chpos].c_style & ST_UL) && ul )
					{
					ul = FALSE;

					t_rule( margin+ul_start, mpt_to_sp(250),
						cpos-ul_start );
					}
				
				cpos += ch_wid( &line[chpos] );
				}
			}
		else
			{

			if( (wd->wd_flags & WD_UL) &&  !ul )
				{
				ul = TRUE;
				ul_start = cpos;
				}
			else
			if( !(wd->wd_flags & WD_UL) &&  ul )
				{
				ul = FALSE;

				t_rule( margin+ul_start, mpt_to_sp(250), 
					cpos-ul_start );
				}

			cpos += wd->wd_length;
			}
		
		}
	if( ul )
		t_rule( margin+ul_start, mpt_to_sp(250), cpos-ul_start );

	}


/*
 *	gimme a transparent rule (assume you are at left margin to start)
 */
t_rule( start, h, w )
SP	start;
SP	h;
SP	w;
	{

	push();

	right( start );

	down( (SP)(h + mpt_to_sp(400)) );

	outcmd( PUTRULE );
	out4( h );
	out4( w );

	pop();

	}


/*
 *	put a character at the current position and advance
 */
set_ch( lptr )
Char	* lptr;
	{
	int	font;

	if( silent )
		return;

	if( (font = fontno(lptr)) != cur_font )
		{

		cur_font = font;
		if( !fonts[font].f_active )
			def_font( font );

		outcmd( FONT0 + font );

		}

	outcmd( SET0 + lptr->c_char );

	cur->p_h += ch_wid( lptr );

	}



/*
 *	measure the size of the footer
 */
init_footer()
	{

	SP	top;

	push();
	silent = TRUE;

	top = cur->p_v = min_to_sp( 500 );

	footer();

	rmargin_v = min_to_sp( 10500 ) - (cur->p_v - top);

	if( verbose )
		fprintf( stderr, "footer length %3.2f in.\n", 
			sp_to_in( cur->p_v - top ) );

	silent = FALSE;
	pop();

	}
	

set_ruler( lmargin, rmargin, indnt, space, just )
PIX	lmargin;
PIX	rmargin;
PIX	indnt;
int	space;
int	just;
	{

	lmargin_h = pix_to_sp( lmargin ) + min_to_sp( 1000 );;
	rmargin_h = pix_to_sp( rmargin ) + min_to_sp( 1000 );;
	indent = pix_to_sp( indnt ) + min_to_sp( 1000 );
	adjust = just;

	switch( space )
		{
		case S_SINGLE:
			spacing = 1.0;
			break;

		case S_ONEANDHALF:
			spacing = 1.5;
			break;

		case S_DOUBLE:
			spacing = 2.0;
			break;

		default:
			warning( "unknown line spacing" );
			spacing = 1.0;
			break;
		}

	}


/*	single level save	*/
LOCAL	SP	o_rmargin_h;
LOCAL	SP	o_lmargin_h;
LOCAL	SP	o_indent;
LOCAL	double	o_spacing;
LOCAL	int	o_adjust;
LOCAL	SP	o_tabs[TABMAX];
LOCAL	int	o_tabcnt;

save_ruler()
	{
	int	i;

	o_rmargin_h = rmargin_h;
	o_lmargin_h = lmargin_h;
	o_indent = indent;
	o_spacing = spacing;
	o_adjust = adjust;

	for( o_tabcnt = tabcnt, i=0; i<o_tabcnt; i++ )
		o_tabs[i] = tabs[i];

	}

restore_ruler()
	{
	int	i;

	rmargin_h = o_rmargin_h;
	lmargin_h = o_lmargin_h;
	indent = o_indent;
	spacing = o_spacing;
	adjust = o_adjust;

	for( tabcnt = o_tabcnt, i=0; i<tabcnt; i++ )
		tabs[i] = o_tabs[i];

	}




clr_tabs()
	{
	tabcnt = 0;
	}

set_tab( tab )
PIX	tab;
	{
	tabs[tabcnt++] = pix_to_sp( tab ) + min_to_sp( 1000 );
	}


cr()
	{
	out_pos( lmargin_h, cur->p_v );
	}


right( h )
SP	h;
	{

	if( h == 0 )
		return;

	outcmd( RIGHT4 );
	out4( h );

	cur->p_h += h;

	}

down( v )
SP	v;
	{

	outcmd( DOWN4 );
	out4( v );

	cur->p_v += v;

	}


/*
 *	go to the indicated position
 */
out_pos( h, v)
long	h;
long	v;
	{

	if( h != cur->p_h )
		{
		outcmd( X4 );
		out4( h - cur->p_h );
		}
	if( v != cur->p_v )
		{
		outcmd( Y4 );
		out4( v - cur->p_v );
		}

	cur->p_h = h;
	cur->p_v = v;

	}

out( outfd, length, value )
FILE	* outfd;
int	length;
long	value;
	{
	REG	int	i;
	int	v;

	for( i=0; i<length; i++ )
		{
		v = value >> (((length-1) - i) * 8);
		putc( v & 0xff, outfd );
		}
	
	}
