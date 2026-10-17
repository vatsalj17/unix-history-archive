#include "ded.h"

int dbug_options 0;

/* options currently available:
 *      a - print out regular expression in build_re
 */

int set_dbug(opt)
char opt;
 { if ('a'<=opt && opt<='a'+15)
     { dbug_options =| (1 << (opt-'a'));
	return(true);
     }
    else
      return(false);
 }

int unset_dbug(opt)
char opt;
 { if ('a'<=opt && opt<='a'+15)
     { dbug_options =& ~ (1 << (opt-'a'));
	return(true);
     }
    else
      return(false);
 }

int dbug(opt)
char opt;
 { if ('a'<=opt && opt<='a'+15)
     return(dbug_options & (1 << (opt-'a')));
    else
      editerror("invalid debug request %c",opt);
 }
