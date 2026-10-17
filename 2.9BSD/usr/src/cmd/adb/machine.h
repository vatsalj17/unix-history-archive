#
/*
 *	UNIX/INTERDATA debugger
 */

/* unix parameters */
#define DBNAME "adb\n"
#define LPRMODE "%Q"
#define OFFMODE "+%o"
#define TXTRNDSIZ 8192L

TYPE	unsigned TXTHDR[8];
TYPE	struct ovhead	OVLVEC;
TYPE	unsigned  SYMV;
TYPE	char  SYMFLG;
TYPE	char  OVTAG;

/* symbol table in a.out file */
struct symtab {
	char	symc[8];
	SYMFLG	symf;
	OVTAG	ovnumb;
	SYMV	symv;
};
#define SYMTABSIZ (sizeof (struct symtab))

#define SYMCHK 047
#define SYMTYPE(symflag) (( symflag>=041 || (symflag>=02 && symflag<=04))\
				?  ((symflag&07)>=3 ? DSYM : (symflag&07))\
				: NSYM\
			)
struct	ovhead {
	unsigned	max;
	unsigned	ov[7];
};
