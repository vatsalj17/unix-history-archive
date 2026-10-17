/*      structure for holding command table info        */

struct cmdstruct
{
    char *cmdname;      /* string reference */
    int  cmdtoken;      /* internal token  */
    int  cmdnargs;      /* minimum argument count */
};

typedef struct cmdstruct Cmd;
