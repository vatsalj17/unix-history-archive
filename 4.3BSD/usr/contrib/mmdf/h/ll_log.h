/*      Definitions for the llog status-recording package                 */

#define LLOGCLS 01		  /* Keep log closed, except when writing */
#define LLOGCYC 02		  /* Cycle to beginning when log full     */
#define LLOGWAT 04		  /* Wait, if log locked and LLOGCLS      */
#define LLOGSOME 07		  /* Occasional logging                   */

#define LLOGFAT  -1		  /* Fatal error                          */
#define LLOGTMP   5		  /* Temporary (minor?) error             */
#define LLOGGEN  10		  /* General information                  */
#define LLOGBST  20		  /* Basic statistics                     */
#define LLOGFST  25		  /* Full statistics                      */
#define LLOGPTR  40               /* Trace of overall program phases      */
#define LLOGBTR  50		  /* Trace of basic program acitivity     */
#define LLOGFTR  55		  /* Full program trace                   */
#define LLOGMAX 126		  /* Maximum possible value               */

struct  ll_struct
{
    char   *ll_file;               /* path name to logging file            */
    char   *ll_hdr;                /* text to put in opening line          */
    int     ll_level;              /* don't print entries higher than this */
    int     ll_msize;              /* max size for log, in 100's of blocks */
    int     ll_stat;               /* assorted switches                    */
    int     ll_fd;                 /* holds file descriptor                */
    FILE   *ll_fp;                 /* Standard IO stream pointer           */
};

typedef struct ll_struct Llog;

typedef struct ll_struct LLog;
