.th GETEQUAL AM 9/12/77
.sh NAME
getequal - get the first tuple equal to the provided key
.sh SYNOPSIS
.nf
getequal(d, key, tuple, tid)
struct descriptor	*d;
char			key[MAXTUP];
char			tuple[MAXTUP];
struct tup_id		*tid;
.fi
.sh DESCRIPTION
GETEQUAL is used to do a keyed retrieval of a single
tuple in cases where the calling program knows the key to
be unique.  SETKEY must be called first to set all desired
domain values.  GETEQUAL will return the first tuple with
equality on all of the specified domains.
The tuple is returned in TUPLE.
.sh DIAGNOSTICS
.nf
AMFIND_ERR -- unidentifiable storage structure in find
AMWRITE_ERR -- pageflush failure
AMREAD_ERR -- get_page failure or invalid error
0 -- success
1 -- no match
.fi
.sh "TRACE FLAGS"
23.8 - 23.10
.sh "SEE ALSO"
clearkeys, closer, find, get, openr, setkey
