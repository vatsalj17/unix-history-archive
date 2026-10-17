#

struct RE
 { int type;
    int info;
 };

/* amount of space that can be taken up by a single string */
#define RESIZE (4*(ENOUGH))

/* direction of search */
#define FORWARD 0
#define BACKWARD 1

/* types of node in an RE structure */
#define p_CHAR          1
#define p_FIN           2
#define p_ANY           3
#define p_BOL           4
#define p_EOL           5
#define p_OPT           6
#define p_STAR          7
#define p_SSTAR         8
#define p_PLUS          9
#define p_PPLUS         10
#define p_INSET         11
#define p_OUTSET        12
#define p_ONEOF         13
#define p_SEP           14

/* definition of 'end-of-line' condition */
#define EOLCOL (4*(ENOUGH))
