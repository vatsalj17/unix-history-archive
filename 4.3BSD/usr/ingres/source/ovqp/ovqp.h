#

/*	@(#)ovqp.h	8.3	2/8/85	*/

/*
**	This header file contains the external (global) declarations
**	of variables and structures as well as the manifest constants
**	particular to OVQP.
**
**	By convention global variable identifiers are spelled with 
**	an initial capital letter; manifest constants are in caps
**	completely.
*/



/*
**	Manifest constants
*/
   
  
# define	tTFLAG		'O'	/* trace flag */

# define	LBUFSIZE	850	/* buffer size for holding query list */
					/* and concat and ascii buffers */
# define	NSIMP		15	/*maximum no. of "simple clauses" 
					 * allowed in Qual list
					 * (see "strategy" portion) */
# ifndef	STACKSIZ
# define 	STACKSIZ	20
# endif
# define	MAXNODES	(2 * MAXDOM) + 50	/* max nodes in De.ov_qvect */

/* symbolic values for GETNXT parameter of fcn GET */
# define	CURTUP	0	/* get tuple specified by tid */
# define	NXTTUP	1	/* get next tuple after one specified by tid */
  

/* symbolic values for CHECKDUPS param of fcn INSERT */
# define	DUPS	0	/* allow a duplicate tuple to be inserted */
# define	NODUPS	1	/* check for and avoid inserting 
				 * a duplicate (if possible)*/


# define	TIDTYPE		INT
# define	TIDLEN		4

# define	CNTLEN 		4	/* counter for aggregate computations */
# define	CNTTYPE 	INT	/* counter type */

# define	OANYLEN		2	/* length for opANY */
# define	OANYTYPE	INT	/* type for opANY */


					 /* (i.e. either De.ov_srcdesc or Ov.ov_indesc) */








typedef char	i1type;
typedef short	i2type;
typedef long	i4type;
typedef float	f4type;
typedef double	f8type;
typedef char	c0type[];


typedef char	*stringp;


/*
**    Structures for string manipulation
**
*/
# define PATNUM 10

typedef struct plist {
    char         *string;
    int		 len;
} PLIST;

typedef struct glist {
    char	*string;
    int		len;
    struct glist *next;
} GLIST;

PLIST   Pats[PATNUM];	/* for use with PAT_SPEC chars in a replace command. */
			/* Holds pattern and corresponding length to be      */
			/* inserted into the new string.  Index of Pats      */
			/* corresponds to index which user types following   */
			/* the PAT_SPEC char.				     */

int     Patnum,		/* Number of patterns to be inserted into replaced   */
			/* string.  Set to zero in endquelst().		     */ 
	Globlen,
	Globnum;
GLIST	*Globs,*Globfoot;

