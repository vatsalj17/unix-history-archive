.th NOCLOSE AM 9/13/77
.sh NAME
noclose - update system catalogs for a relation
.sh SYNOPSIS
.nf
noclose(d)
struct descriptor	*d;
.fi
.sh DESCRIPTION
NOCLOSE is used to update the system catalogs
for a relation just as CLOSER does,
but without closing the relation.
If a program repeatedly OPENR's and CLOSER's
a relation so that the system catalogs will be
up-to-date for use by other programs,
NOCLOSE can be used to eliminate the overhead
of physically opening and closing relations
and still achieve the same results.
.sh DIAGNOSTICS
.nf
AMWRITE_ERR -- pageflush failure
AMREAD_ERR -- get_page failure
0 -- success
1 -- relation was not open
.fi
.sh "TRACE FLAGS"
21.12
.sh "SEE ALSO"
closer, openr
