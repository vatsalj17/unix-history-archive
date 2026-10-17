.\" tbl dtbl.1 > dtbl.t.1
.TH DTBL 1 
.SH NAME
dtbl \- format tables for 
.SH SYNOPSIS
.B dtbl
[ files ] ...
.SH DESCRIPTION
.I Dtbl
is a preprocessor for formatting tables for
.IR ditroff (1).
The input files are copied to the standard output,
except for lines between .TS and .TE
command lines, which are assumed to describe tables
and reformatted.
Details are given in the reference manual.
.PP
As an example, letting \\t represent a tab (which should
be typed as a genuine tab)
the input
.IP ""
\&.TS
.nf
c s s
c c s
c c c
l n n.
\_
Household Population
Town\\tHouseholds
\\tNumber\\tSize
\_
Bedminster\\t789\\t3.26
Bernards Twp.\et3087\et3.74
Bernardsville\et2018\et3.30
Bound Brook\et3425\et3.04
Branchburg\et1644\et3.49
Bridgewater\et7897\et3.81
Far Hills\et240\et3.19
\_
\&.TE
.LP
.fi
yields
.ne 10
.IP " "
.TS
c s s
c c s
c c c
l n n.
_
Household Population
Town	Households
	Number	Size
_
Bedminster	789	3.26
Bernards Twp.	3087	3.74
Bernardsville	2018	3.30
Bound Brook	3425	3.04
Branchburg	1644	3.49
Bridgewater	7897	3.81
Far Hills	240	3.19
_
.TE
.fi
.PP
If no arguments are given,
.I dtbl
reads the standard input,
so it may be used as a filter.
When it is used with
.I deqn
the
.I dtbl
command should be first, to minimize the volume
of data passed through
pipes.
.SH SEE ALSO
ditroff(1), deqn(1)
.br
M. E. Lesk,
.I TBL.
