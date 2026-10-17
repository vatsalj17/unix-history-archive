#define true 1
#define false 0


/* screen size parameters - now set as variables
 * from the characteristics of the terminal (see
 * ttysetup procedure)
 */
extern int nrows;

#define NINROWS         (nrows-1)
#define LASTINROW       (nrows-2)
#define EDITROW         (nrows-1)
#define FOOTROW         (nrows-1)
#define SCROLL          (nrows/3)

extern int ncols;

#define BELLMARGIN      (ncols/5*4)     /* 64 chars per line */
#define ENOUGH          128             /* enough for one screen line */

/* size of file */
#define NLINES 2000


/* input character classes */
#define IGN 0
#define ONE 1
#define TWO 2
#define TAB 3
#define MODIFY 4
#define SPACE 5
#define CR 6
#define NL 7
#define WLEFT 8
#define WRIGHT 9
#define LEFT 10
#define RIGHT 11
#define UP 12
#define DOWN 13
#define CONTROL         14

/* characters which are commands */
#define SPECIAL         20      /* introduce special character interpretation */
#define MODEetoi        21      /* change mode and do command */
#define CHMODE          22      /* change mode (input <-> command */

#define RUBOUT          23              /* erase PREVIOUS character */
#define CERASE          24              /* erase PRESENT character */
#define WRUBOUT         25              /* erase PREVIOUS word */
#define WERASE          26              /* erase PRESENT word */
#define HEADERASE       27              /* erase row up to cursor */
#define TAILERASE       28              /* erase row from cursor */
#define MODEitoe        29              /* into edit mode and erase line */

/* more input character classes */
#define LSTART          100             /* start of line */
#define LEND            101             /* end of line */


/* editor modes */

#define INMODE 0
#define EDMODE 1

/* terminal types */
#define itt 0
#define hazeltine 1
#define VC404 2
#define T1061 3

/* define your terminal here *******************/
#define TTY VC404
/* structure definitions */

struct iobuf
 { int fildes;
    int nleft;
    char *nextp;
    char buf[512];
 };

struct CURSOR
 { int row;
    int col;
 };


/*
 * external structures
 */
extern struct iobuf tmp_buf;
extern struct CURSOR virt_c,real_c;
extern struct CURSOR in_c, edit_c;
extern char **rowmap, **auxscreen;
extern int l_block[NLINES], l_offset[NLINES];
extern char f_cache[512];
extern char ictab[128], ectab[128];
extern char filename[128], tempname[128];
extern char pidname[128], pathname[128];
extern char tailname[128];
extern char dlogname[128];
extern char erow[128];
extern int b_range[26]; e_range[26];

/*
 * external variables
 */

extern int topl, maxl;
extern int txin, tbout;
extern int ringing;
extern int mode;
extern int toprow,bottomrow,leftcol,rightcol;
extern int ms_line, ms_col, mf_line, mf_col;
extern int tmp_changed;
extern int filewritten;
extern int dlogger, tty_input;
extern int setup_done; /* ttysetup */
extern int ttytype;
extern int modopt; /* to say whether a modifier has been selected */

/*
 * external functions
 */

extern itoe(), etoi();
extern mainloop();
extern int do_command();
extern int set_dbug(), unset_dbug(), dbug();
extern ttyinterrupt();
