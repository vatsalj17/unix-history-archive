.th CLOSER AM 9/6/75
.sh NAME
closer - close a relation
.sh SYNOPSIS
.nf
closer(d)
struct descriptor	*d;
.fi
.sh DESCRIPTION
CLOSER is used to close a relation which was opened by OPENR.
If the number of tuples in the relation were changed by the program,
CLOSER will update the RELTUPS field in the relation relation.
If all relations are opened for reading only,
(i.e. the number of tuples could not be changed)
it may be desirable to use INPCLOSER instead of CLOSER.
CLOSER should always be called once for each OPENR.
.sh DIAGNOSTICS
.nf
AMCLOSE_ERR -- cannot close the relation
AMWRITE_ERR -- pageflush failure
AMREAD_ERR -- get_page failure
0 -- success
1 -- relation was never open
.fi
.sh "TRACE FLAGS"
21.0
.sh "SEE ALSO"
inpcloser, noclose, openr
