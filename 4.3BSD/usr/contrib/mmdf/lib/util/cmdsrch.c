#include "util.h"
#include "cmd.h"

extern int errno;

cmdsrch (str, argc, cmd)        /* find command. return token reference */
    char str[];                 /* test string  */
    int argc;                   /* number of available arguments        */
    register Cmd *cmd;          /* table of known commands */
{
    for ( ; (cmd -> cmdname) != (char *) 0; cmd++)
	if (lexequ (str, cmd -> cmdname))
	{                       /* got a hit            */
	    if (argc < cmd -> cmdnargs)
	    {
		errno = EINVAL;
		return (NOTOK);
	    }
	    return (cmd -> cmdtoken);
	}

    return (NO);
}
