#define TTYBUFSIZE 512

struct TTYBUF
 { int n_in;
    char *nxfree;
    char tbuf[TTYBUFSIZE];
 };


/*
 * tty modes
 */
#define T_ITT   0100000
#define T_RARE  040000
#define T_INDCTL 01000
#define T_RAW   040
#define T_CRLF  020
#define T_ECHO  010
#define T_TABS  02
