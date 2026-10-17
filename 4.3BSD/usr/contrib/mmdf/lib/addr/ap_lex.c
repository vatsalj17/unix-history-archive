#include "util.h"
#include "ap.h"
#include "ap_lex.h"

/*  Perform lexical analysis on input stream

    The stream is assumed to be "unfolded" and the <crlf>/<lwsp> sequence
    is NOT checked for.  This must be done by the character-acquisition
    routine, if necessary.  In fact, space, tab and newline all return the
    same lexical token.  Due to a number of bagbiting mail systems on the
    net which cannot handle having a space within a mailbox name, period
    (.) has been equated with space.

    Letters, numbers, and other graphics, except specials, also all return
    the same token.

    Note that only printable characters and format effectors are legal.
    All others cause an error return.

    Only COMMENTs and WORDs have data associated with them.

*/

/*  < 1978  B. Borden       Wrote initial version of parser code
 *  78-80   D. Crocker      Reworked parser into current form
 *  Apr 81  K. Harrenstein  Hacked for SRI
 *  Jun 81  D. Crocker      Back in the fold.  Finished v7 conversion
 *                          minor cleanups.
 */

extern char ap_lxtable[];         /* ascii chars -> symbolic terminals    */
extern int ap_intype;
int     ap_peek = -1;             /* one-character look-ahead             */
char    ap_llex;                  /* last lexeme returned by ap_lex ()    */

#if DEBUG > 1
extern char debug;
char   *namtab[] =
{
    "eo-data",                    /* LV_EOD          0                    */
    "error",                      /* LV_ERROR        1                    */
    "comma",                      /* LV_COMMA        2                    */
    "at",                         /* LV_AT           3                    */
    "colon",                      /* LV_COLON        4                    */
    "semi",                       /* LV_SEMI         5                    */
    "comment",                    /* LV_COMMENT      6                    */
    "less",                       /* LV_LESS         7                    */
    "grtr",                       /* LV_GRTR         8                    */
    "word",                       /* LV_WORD         9                    */
    "from",                       /* LV_FROM        10                    */
    "domain-literal"              /* LV_DLIT        11                    */
};
#endif

/**/

ap_lex (lexval)
char    lexval[];
{
    register char   c,
		   *lexptr;
    register int    retval;

    while ((retval = ap_lxtable[c = ap_char ()]) == LT_SPC)
	;                         /* Skip space, tab and newline          */
    lexptr = lexval;
    *lexptr++ = c;

    switch (retval)
    {
	case LT_ERR:              /* Bad Character */
	    retval = LV_ERROR;
	    break;

	case LT_EOD:              /* End Of Data stream */
	    retval = LV_EOD;
	    break;

	case LT_COM:              /* comma ","  -- addr list separator    */
	    retval = LV_COMMA;
	    break;

	case LT_AT:               /* At sign "@"  -- node separator       */
	    retval = LV_AT;
	    break;


/* *******************  DATA TYPES AND GROUP LIST  ******************** */

	case LT_COL:              /* colon  ":" -- data type / group      */
	    retval = LV_COLON;
	    break;

	case LT_SEM:              /* semicolon ";" -- group end           */
	    retval = LV_SEMI;
	    break;



/* ***********************  PERSON ADDRESS LIST  ************************ */

	case LT_LES:              /* less-than-sign "<"  -- person list   */
	    if (ap_lxtable[c = ap_char ()] == LT_LES)
		retval = LV_FROM; /* << implies redirection               */
	    else {
		ap_peek = c;      /* restore xtra char                    */
		retval = LV_LESS;
	    }
	    break;

	case LT_GTR:              /* greater-than-sign ">" -- end person  */
	    retval = LV_GRTR;
	    break;
/* *******************  QUOTED & UNQUOTED WORDS  ********************** */

	case LT_LTR:              /* letters                              */
	case LT_SQT:              /* single-quote "'"  -- just char, here */
	case LT_RPR:              /* right paren ")" -- just char, here   */
	    for (;;) {
		switch (ap_lxtable[*lexptr++ = c = ap_char ()]) {
		    case LT_LTR:
		    case LT_SQT:
		    case LT_RPR:
			continue;

		    case LT_ERR:
			retval = LV_ERROR;
			break;

		    case LT_EOD:  /* permit eod to end string            */
		    default:      /* non-member character                 */
			ap_peek = c;
			lexptr--;
#ifdef notdef /* no more " at " == '@' */
			if (ap_intype == AP_733 &&
			      lexptr == &lexval[2] &&
				uptolow (lexval[0]) == 'a' &&
				uptolow (lexval[1]) == 't'   )
			    retval = LV_AT;
			else
#endif notdef
			    retval = LV_WORD;
		}
		break;
	    }
	    break;

	case LT_QOT:              /* double quote "\""  => string         */
	    retval = LV_WORD;
	    --lexptr;           /* don't put quotes into obvalue  - SEK   */
	    for (;;) {
		switch (ap_lxtable[*lexptr++ = c = ap_char ()]) {
		    case LT_QOT:
			--lexptr;
			break;
		    case LT_SQT:  /* include next char w/out interpeting  */
				  /* and drop on through                  */
			*(lexptr - 1) = ap_char ();
		    case LT_RPR:
		    case LT_LPR:
		    default:
			continue;
		    case LT_ERR:
		    case LT_EOD:
			retval = LV_ERROR;
		}
		break;
	    }
	    break;
/* *************************  COMMENT  ******************************** */

	case LT_LPR:              /* left paren "("  -- comment start     */
	    lexptr--;             /* remove left-most paren */
	    for (retval = 0;;) {  /* retval is count of comment nesting   */
		switch (ap_lxtable[*lexptr++ = c = ap_char ()]) {
		    case LT_LPR:  /* nested comments                      */
			retval++; /* just drop on through                 */
		    default: 
			continue;
		    case LT_SQT:  /* include next char w/out interpeting  */
			*(lexptr - 1) = ap_char ();
			continue;
		    case LT_RPR: 
			if (--retval > 0)
			    break;
			lexptr--;       /* remove right-most paren */
			retval = LV_COMMENT;
			break;
		    case LT_EOD: 
		    case LT_ERR: 
			retval = LV_ERROR;
			break;
		}
		break;
	    }
	    break;


/* *********************  DOMAIN LITERAL  ***************************** */

	case LT_LSQ:              /* left squar bracket "["               */
	    FOREVER
	    {
		switch (ap_lxtable[*lexptr++ = c = ap_char ()])
		{
		    default: 
			continue;
		    case LT_SQT:  /* include next char w/out interpeting  */
			*(lexptr - 1) = ap_char ();
			continue;
		    case LT_RSQ:
			retval = LV_DLIT;
			break;
		    case LT_EOD: 
		    case LT_ERR: 
			retval = LV_ERROR;
			break;
		}
		break;
	    }
	    break;
    }


/* ***********************  CLEANUP AND RETURN  ************************* */

    *lexptr = '\0';

#if DEBUG > 1
    if (debug)
	printf (" %s", namtab[retval]);
#endif

    return (ap_llex = retval);
}
/* *******************  GET NEXT INPUT CHARACTER  ********************* */

ap_char ()
{                                 /* handle lookahead and 8th bit         */
    extern int  (*ap_gfunc) ();   /* Ptr to character get fn              */
    register int    i;

    if (ap_peek == 0)
	return (0);
    if ((i = ap_peek) > 0) {
	ap_peek = -1;
	return (i);
    }

    if ((i = ((*ap_gfunc) ())) == -1)
	return (0);               /*  EOD                                 */

    return ((isascii (i)) ? i : '\177');
				  /* force error, if eighth bit is on     */
}

