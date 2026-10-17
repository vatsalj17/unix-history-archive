.pl 72
.RP
.tr ~~
.tc
.ND
.sp 5
.TL
M A C
.sp 2
Multiple Assembly-language
Compiler
.sp 3
.AU
Ross Nealon
.AI
University of Wollongong.
.AB
MAC is a generalised cross-assembler,
table driven, with a finite-state parsing
algorithm built in. MAC accepts a description
of the target machine's architecture, a
list of the symbolic opcodes and their values,
and a list of bitwise instruction formats.
MAC generates output in three forms -
listings, code dumps, and loader format object files.
MAC comes equipped with a table-formatting
program, to format r-files from symbolic
descriptions.
.sp
.ti +3
This document comprises the first part
of the description of the MAC cross-assembler.
The second part describes the table formatter
for producing a target machine description for MAC,
and the third part describes the system from an implementors
point of view.
.AE
.NH 1
GENERAL DESCRIPTION
.PP
MAC is a two-pass finite state assembler
that accepts symbolic assembly-language
statements, and performs a one-to-one
translation, compiling each instruction into
its equivalent binary machine code.
.PP
MAC will read source statements until
end-of-file, or until an 'end' directive is
encountered.
Any error in the first pass will cause termination
of the assembly at the end of the first pass.
.PP
Pass one is devoted to scanning the source line,
performing syntax checking, re-coding the source and
building a file of intermediate coded source (intercode)
one record per source line, and building the symbol
table, a list of labels and their values.
.PP
Pass two re-reads the intercode records and uses the
entries in the symbol table to generate the binary
machine code, and produce useable output in the
form of listings, code dumps, and loader format
object files.
These formats are described in a later section of this manual.
.PP
The programmer wishing to use MAC must supply MAC with
the name of a pre-formatted file, containing the
description of the target, the parser table and so on.
These pre-formatted files (r-files) reside in the
library '/usr/lib/mac'.
It is only necessary to provide the name of a file
in the library,
as MAC first searches the current directory,
then the library for the file named.
.PP
MAC generates listings by re-reading
the source code, so that an assembly of source
from the standard input cannot generate a listing.
.bp
.NH 1
SYNOPSIS
.sp
.PP
mac [-opts [...]] r-file [source] [object]
.sp 2
.po +8
.IP "opts:" 12
Options appear one per argument.
.sp
-l:  Generate source/code listing
.br
-d:  Generate code dump
.br
-a:  Generate loader object file
.br
-s:  Print symbol table
.br
-f:  Don't use form-feeds
.br
-h:  Data output in Hexadecimal (default)
.br
-o:  In octal
.br
-b:  In binary
.br
-u:  Do not unlink temp file
.br
-e:  Supress ALL error messages
.sp
.IP "r-file:" 12
Name  of a file  containing a set of
pre-formatted tables for MAC.
.sp
.IP "source:" 12
The name of a file containing the source
to be assembled.  If not present, MAC will
read from the standard input.
.sp
.IP "object:" 12
If present, MAC assumes '-a' option, and
generates a loader format object file
with that name.
If '-a' is on and this argument is not
present, MAC uses the name "a.out".
.sp 2
.IP "examples:" 12
mac -l -s m6800 vdu.s vdu.o
.br
mac z80 t.s >dump
.br
mac -a -n 8080 copy.s
.br
.po -8
.bp
.NH 1
SOURCE LINE
.sp
.PP
[label]  [opcode [args]]  [;comment]
.sp 2
.po +8
.IP "label:" 12
A previously non-defined label can tag
the start of each source line. The label
is then defined to have the value of the current
location counter.
The label must start in column one,
otherwise it is treated as an opcode.
.sp
.IP "opcode:" 12
A special operation code symbolic or a
pseudo opcode to indicate what code to
generate. If this field is not present,
the args field is not allowed.
.sp
.IP "args:" 12
This field is made up of expressions,
literals, special characters and delimiters,
with no intervening spaces or tabs. This
field adds to the information describing
the code to be generated.
This field is often called the
argument "picture".
.sp
.IP "comment:" 12
All characters after a ';' (not in a
character constant) are treated as a comment
and are ignored.  A comment
terminates scan of the source line.
.sp 2
.PP
All intervening white-space separating fields
on the source line may be any number
of blanks and/or tabs.
.br
.po -8
.bp
.NH 1
LITERALS
.PP
A literal is a label, which has been defined (in
the r-file) as reserved. This label cannot be
used in expressions, as it has no value.
It cannot be set to a value.
Literals are useful only to recognise argument
pictures on instructions.
Refer to the description of the r-file
that you will be using for a list of defined
literals.
.sp 2
.NH 1
LABELS
.PP
A label is from one to eight lowercase alpha-alpha/numeric
characters. The first character must be from the set
{a-z @ _ .}
and may optionally be followed by one to seven
alpha/numerics {a-z @ _ . 0-9}.
.sp
.IP "examples:" 12
.sp
ll1     .sin     .mul
.br
@reg1   __acc    _._
.br
ret     loop1    ent27
.br
.sp 2
.NH 1
OPERATORS
.PP
Mac recognises the following operators:-
.sp
.po +12
.DS
+     addition  (binary or unary)
-     subtraction  (binary or unary)
*     multiplication
/     division
%     modulus
|     bit-wise logical or
&     bit-wise logical and
\~     exclusive or
>     right-shift
<     left-shift
.DE
.po -12
.sp 2
.PP
The unary operators plus (+), minus (-), and
complement
(\~)
may be added to prefix any term
in an expression.  Note that only one unary
operator per term is allowed.
.bp
.NH 1
EXPRESSIONS
.PP
An expression is an unparenthesized
list of labels, constants, location
counter symbols (!) called terms; and
operators.
An expression must consist of at least
one term, optionally followed by
one or more operator-term pairs.
Any term may be optionally prefixed
by a unary operator.
.sp 2
.PP
EXAMPLES:
.sp 2
.DS
ll+1
entr-adc+isp/4
entr-adc+isp>2
-mask|e_bit
.sp 2
NOTE:
        2+3*4   is evaluated as (2+3)*4,
                not 2+(3*4).
.DE
.bp
.NH 1
CONSTANTS
.PP
Numeric constants can be described as C-type
constants.  A constant beginning with {1-9} is
interpreted as decimal, beginning with 0 is
interpreted as octal,
beginning with 0x as hexadecimal, and
0b as binary.
.sp
.DS
    examples:-
.sp
        123     55      91234        (decimal)
        0       0666    077777       (octal)
        0xf618  0xff    0x34ac       (hex)
        0b101   0b111011011011       (binary)
.DE
.sp
.PP
Negative constants are obtained by combining a positive
constant with the unary negation operator (-).
.sp
.PP
Character constants are enclosed in single quotes,
and are treated as small integer constants,
with their values being made up of a concatenation
of their respective ASCII values.
The C-language escape conventions apply,
\\n => newline, \\r => carriage return,
\\f => form-feed, \\b => backspace,
\\t => tab, and \\0 => ASCII NUL.
.sp
.PP
Strings are enclosed in double quotes, and have no
real numeric value. They cannot be
used in expressions. The characters of the string
are assembled one per consecutive byte in memory.
Strings are only useful as title information
(See later - pseudo opcode 'title') and
for the special definition of constants
(See later - pseudo opcode 'dc').
.PP
Strings are limited to thirty characters in length,
but space exists in the listing to display ten.
Any more than ten characters,
and the remainder will not be listed.
This does not affect the code generated.
.bp
.NH 1
PSEUDO OPCODES
.sp 2
.PP
A special type of opcode symbolic is the pseudo
opcode. These are always defined to the assembler,
and provide the user with the means to control
the location counters, their values, code generation,
constant and storage definition and listings.
.sp 2
.po +8
.IP "eject:" 12
If a listing is being generated,
skip to the top of a new page and
output title and header information.
.sp
[label]     eject
.sp
.IP "title:" 12
Set title information, and perform 'eject's function.
.sp
[label]     title   "[string]"
.br
.sp
.IP "end:" 12
End of source code indicator. This will
terminate pass one, check the symbol table
for errors (undefined labels etc.) and reset
various states within the assembler in
preparation for pass two.
Any source code present after an 'end' directive
will be ignored.
.sp
[label]   end
.sp
.IP "seg:" 12
Seg selects a particular location counter to use.
MAC comes equipped with eight distinct location
counters, any of which may be selected.
Each location counter will generate
a distinct segment of code.
Location counter 0 is the default.
Segments may be interleaved within the source
code, MAC will assemble each segment distinctly.
Each segment exists exclusively of any others.
.sp
[label]   seg     expression
.sp
.IP "equ:" 12
Equ equates the label tag to the defined expression.
This expression must be defined before this
instruction is processed.
.sp
label     equ     expression
.bp
.IP "org:" 12
Org (abbreviation for origin) is used to set the current
location counter's value.
.sp
[label]   org     expression
.sp
.IP "align:" 12
Align sets the current location counter
to the next even multiple of the argument
expression. No alignment is performed
if it is not necessary.
.sp
[label]   align   expression
.sp
.IP "global:" 12
This is only useful
when an object file is being generated,
as any global label is dumped with the
code in a symbol table at the end of the object file.
.sp
[label1]  global  label2
.sp
.IP "ds:" 12
Defines a number of null bytes of storage
specified by the argument expression.
.sp
[label]   ds      expression
.sp
.IP "dc:" 12
Define constant allows the definition
of a constant in memory with a value equal
to the argument expression.
The format of the constant in memory
is dependant upon the format described in
the r-file. Refer to the write-up
on the r-file that you will be using.
If the argument is a string, then each
character of the string is assembled
(its ASCII value) into consecutive bytes
of memory.
.sp
[label]   dc      expression
.br
[label]   dc      "string"
.bp
.IP "struc:" 12
Struc allows the user to create "structures"
that are really labels equated to offsets
from the start of the structure.
The general form of struct is a label, and
the storage in bytes for the label.
The label will be set to the value of the 
current offset, and the offset counter
then incremented by the storage length.
This is equivalent to the C
struct declaration.
If the label is omitted, then the
structure offset counter is incremented.
.sp
[label]   struc   expression
.sp
.IP "ends:" 12
Ends equates the label tag (if given)
to the value of the structure offset
counter and then resets the counter to zero.
The next struc pseudo op will therefore
start a new structure definition.
If the label tag is present, it will be
given a value corresponding to the size of the
structure in bytes.
.sp
[label]   ends
.sp 2
.po -8
.PP
Up to four other special define constant pseudo
opcodes can be declared in the r-file.
These may exist to define funny length
constants (e.g.- double word) or funny format
constants (e.g.- two bytes, with the bytes swapped).
The user should consult
the description of his r-file, to find the
exact nature and name of each of the dc's.
Generally - they are of the form dc?, where
? is any legal alpha/numeric.
.bp
.NH 1
ERRORS
.sp 2
.PP
MAC generates three types of error messages:
non-fatal warnings, severe errors that will
cause incorrect code generation, and fatal
internal errors.
.PP
No action is taken on warnings, severe errors
cause termination of the current pass (one or two),
and fatal errors cause immediate termination of
the assembly.
The error messages appear before the
line in error.
.sp 2
.NH 2
WARNINGS
.IP "assemble overflow" 5
.br
An expression has been assembled which
is numerically too large to
fit into the assigned space in the instruction.
Check the r-file write-up as to the
exact format of the instruction in error.
.IP "Listing from std. input impossible!!" 5
.br
Since MAC re-reads the source code to generate
a listing, re-reading from the standard input
is impossibe, hence no listing can be generated.
The -l option is turned off.
.IP "no end stmt" 5
.br
End-of-file has been encountered,
no end directive seen.
.sp 2
.NH 2
SEVERE
.IP "bad argument" 5
.br
Part of an expression is not a label,
a constant or the location counter symbol '!'.
(Possible control character.)
.IP "dc not allowed" 5
.br
The special dc pseudo op (with no
identifying character) has been used
when not declared in the r-file.
Check the r-file write-up.
.IP "delimiter unexpected" 5
.br
Some delimiter has been encountered
unexpectedly in an expression.
.IP "div by zero" 5
.br
An attempt to divide by zero has been
trapped.  Check validity of the
expression(s) on the instruction argument(s).
.IP "expression required" 5
.br
An expression is required as part of the
instruction's arguments.
.bp
.IP "illegal instruction" 5
.br
An attempt has been made to assemble an
instruction that has no legal opcode value.
This means that the argument picture
used on this instruction is illegal,
this instruction cannot have this
format of arguments.
.IP "label required" 5
.br
A label is required for the global
pseudo op.
The label must appear in the argument picture,
and must not be part of an expression.
.IP "label tag required" 5
.br
A label tag starting in column one
is required here.
.IP "label undefined" 5
.br
An attempt has been made to use a label
in an expression, but as yet is has not been
defined and has no value.
.IP "missing argument" 5
.br
An expression is expected in the argument field,
but none was found.
.IP "mod by zero" 5
.br
An attempt has been made to find the value of an
expression modulus zero.
.IP "multi def. label" 5
.br
The label tag on this line has been
previously defined.
.IP "negative ds" 5
.br
The result of the expression argument
to the ds pseudo instruction is negative.
MAC cannot define a negative number of
storage bytes.
.IP "negative org" 5
.br
The location counters cannot be set to
a negative value.
.IP "no such location counter" 5
.br
The argument expression to the seg
pseudo op is not in the allowable range.
(0 to 7 currently).
.IP "op not found" 5
.br
The opcode symbolic on this line is not a legal
symbolic for this r-file. Check the r-file
write-up.
.IP "syntax error" 5
.br
The argument picture for this instruction
is syntactically incorrect.
(E.g.- two adjacent operators, no delimiters
separating expressions, etc.)
.IP "title not a string" 5
.br
The argument to the title pseudo opcode
must be a string. Null strings ("")
turn the title off.
.bp
.IP "Undefined labels" 5
.br
Self explanatory.
.IP "wrong # of args" 5
.br
The instruction being assembled requires more
or fewer expressions in the argument picture
than have been given. Consult the r-file write-up.
.sp 2
.NH 2
FATAL
.IP "buffer overflow" 5
.br
More than the maximum buffer size of characters
has been typed on one line.
.IP "Can't create a.out" 5
.br
A.out file exists and is protected, or cannot be
created in this directory.
No object is generated.
.IP "Can't create object file" 5
.br
As for a.out.
.IP "Can't create temp" 5
.br
The temporary file for intermediate source
code cannot be created in this directory,
or one exists and is protected.
.IP "Can't open r-file" 5
.br
MAC cannot locate the named r-file.
Check that it exists in your directory,
or in /usr/lib/mac.
.IP "Can't re-open source" 5
.br
Something has happened to the source file
since it was last read. A listing cannot be
generated.
.IP "Can't re-open temp" 5
.br
This is similar to re-open source,
but causes MAC to halt.
.IP "Can't find source file" 5
.br
Input source file cannot be opened.
.IP "corrupted format descriptor <item>" 5
.br
This means that the r-file in use
has been corrupted. Report this error
as soon as practical.
.IP "Errors in pass 1." 5
.br
Self explanatory. Pass two is inhibited.
.IP "internal error mode <scan-mode>" 5
.br
MAC's internal table lookup routines
have been called in error.
Report this as soon as practical.
.bp
.IP "label <name> is undefined" 5
.br
The named label has been
referenced in the source program,
but never defined.
.IP "no core for assembly" 5
.br
The program being assembled is
so large that it cannot be assembled in the
host machines memory.
(Split the program into smaller pieces,
and assemble each independantly.)
.IP "Pass 1 non-existant action" 5
.br
MAC's internal pass one parser table
or MAC itself is bad. Report this as soon as is practical.
.IP "Symbol table overflow." 5
.br
Too many labels are being defined.
MAC cannot get enough of the host machine's
memory to define them all.
.IP "Usage: <name> opcode-file [source] [object]" 5
.br
Incorrect parameters on the call.
Usually r-file missing.
.bp
.NH 1
LISTING FORMAT
.sp
.PP
loc.-counter   code   line-#   source
.sp
.PP
The location counter field displays
the value of the location counter before
the next instruction is assembled.
.PP
The code field is the actual assembled code
from the following source instruction.
When using 'dc' to assemble strings, any
more than ten characters per string will
cause MAC to truncate the listing of the string
to ten characters, and not list the remaining
characters of the string. This does not affect
the code being generated in an object file.
For listing purposes, it is best to define long strings as
several short strings.
.PP
The line number is the source line number
and is useful for locating lines in
error.
.PP
The source field is the actual source code as
seen by MAC.
.sp 3
.NH 1
DUMP FORMAT
.sp
.PP
Code is dumped to the standard output
with the segment number, start address and
segment length.
The code is then dumped in the default format
(hex, octal or binary).
.sp 3
.NH 1
OBJECT FILE FORMAT
.sp 1
.PP
The format of object files is generally unimportant,
as several linkage editors or loaders exist for
the various machines that MAC can currently assemble
code for.
The user is therefore advised to consult the
manual concerning the particular
loader that he or she will use.
.bp
