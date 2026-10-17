#define FS '\034'
#define MAXVAL 20       /* allow this many values per entry */
#define ENTRYSIZE 1024

typedef struct DBvalues
{
    char *RECname;      /* name of table */
    char *RECval;       /* value for this record, in this table */
}       DBMValues[MAXVAL + 1];

