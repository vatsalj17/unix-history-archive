.th DELETE AM 9/6/77
.sh NAME
delete - delete a tuple
.sh SYNOPSIS
.nf
delete(d, tid)
struct descriptor	*d;
struct tup_id		*tid;
.fi
.sh DESCRIPTION
DELETE will delete the tuple indicated by TID
from the relation specified by D.
.sh DIAGNOSTICS
.nf
AMINVL_ERR -- invalid tid specified.
AMREAD_ERR -- get_page or invalid function error
0 -- success
2 -- tuple already deleted
.fi
.sh "TRACE FLAGS"
24.0
.sh "SEE ALSO"
del_tuple, get, insert, openr
