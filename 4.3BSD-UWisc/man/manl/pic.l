.TH PIC 1 
.SH NAME
pic \- troff preprocessor for drawing simple pictures
.SH SYNOPSIS
.B pic
[
.BI \-P t
]
[ files ]
.SH DESCRIPTION
.I Pic
is a
.I ditroff
preprocessor for drawing simple figures on a typesetter.
The basic objects are
.IR box ,
.IR line ,
.IR arrow ,
.IR circle ,
.IR ellipse ,
.IR arc
and text.
.PP
The optional argument
.BI \-P t
specifies output device
.IR t ;
currently supported typesetters are
.I va
(or varian, the default),
.I vp
(or versatec),
.I ip
(or imagen).
.SH "SEE ALSO"
B. W. Kernighan,
.I "PIC \(em A Graphics Language for Typesetting"
ditroff(1), grn(1), ideal(1)
