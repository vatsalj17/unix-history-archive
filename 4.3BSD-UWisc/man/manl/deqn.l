.TH DEQN 1
.SH NAME
deqn  \-  typeset mathematics
.SH SYNOPSIS
.B eqn
[
.BR \-d xy
] [
.BR \-p n
] [
.BR \-s n
] [
.BR \-f n
] [
.BR \-P dest
] [ file ] ...
.br
.B checkeq
[ file ] ...
.SH DESCRIPTION
.I Deqn
is a
.IR ditroff(1)
preprocessor
equivalent to eqn but capable of producing input for
.I ditroff
instead of
.IR vtroff .
Usage is almost always
.PP
	eqn file ... | ditroff
.PP
If no files are specified, 
.I deqn
reads from the standard input.
.I Deqn
prepares output for the typesetter 
named in the
.B \-P
option (Varian/Versatec default, see
.IR ditroff (1)).
.SH "SEE ALSO"
.PP
eqn(1), ditroff(1), dtbl(1)
