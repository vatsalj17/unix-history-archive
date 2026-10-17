.pl 72
.ND
.sp 5
.TL
MACTAB
.sp
Multiple Assembly-language Compiler
.br
Table Formatter.
.sp 2
.AU
Ross Nealon
.sp
.AI
University of Wollongong.
.bp
.NH 1
GENERAL DESCRIPTION.
.PP
MACTAB is one of the companion programs to the
MAC cross-assembler.
Its purpose is to aid the user in the
formatting and production of a description file
for the MAC cross-assembler.
.PP
This file is expected to be an absolute
binary data file, containing a concise description
of the target machines architecture, the desired
format of the assembly-language source line, and
other necessary details.
.PP
All error messages are reported to the standard
output, and are flagged with the line number of the
description line in error.
Any error will abort production of the description
file.
.bp
.NH 1
TERMINOLOGY.
.IP "r-file:  " 12
An r-file is a file containing a formatted
description of a machine for the MAC cross-assembler.
This is MACTAB's only
useful output.
.IP "opcode:  " 12
An opcode symbolic is the symbolic
label chosen to represent a particular
instruction. Thus - 'sub' may be chosen
to mean the subtraction instruction.
.IP "class:  " 12
The opcode class is a number, which describes
to which category of instructions this
particular symbolic belongs.
Classes may represent addressing modes, or
instruction types, such as branches, loads
and stores.
.IP "descriptor:  " 12
A format descriptor is a concise description
of a binary instruction, giving the location
of the argument fields within the instruction,
and the width of the fields.
There must exist one format descriptor
for each possible binary instruction format.
.IP "picture:  " 12
The argument picture is the part of the source line
after the symbolic opcode. The picture
contains expressions, which are evaluated to
form arguments to the instruction being
assembled.
The picture can be used to recognise different
classes of opcodes.
Thus - '#123' might mean immediate mode,
while '123' might mean the contents of address 123.
.IP "labels:  " 12
Labels are defined as standard MAC labels,
consisting of at least one alpha character,
from the set {a-z . _ @} and can optionally
be followed by one to seven alphanumerics, from
the set {a-z . _ @ 0-9}.
MACTAB allows the user to pre-define labels,
that is - to assign a value to a label.
This label thereafter cannot be redefined,
and has its assigned value in any expression.
.IP "literals:  " 12
Literals are special labels, that are defined as reserved,
and have no value. Literals cannot be used in expressions,
but are useful for recognising different argument pictures.
Thus, if 'x' is defined as literal, the picture 'expr,x'
would indicate that an expression then comma then literal 'x'
are required in that order for a succesful parse of that
source line.
.bp
.NH 1
SYNOPSIS.
.PP
mactab   source   r-file
.sp 2
.IP "    source:  " 12
Input description source code.
.IP "    r-file:  " 12
Output filename.
.sp 2
.PP
Both arguments are required.
MACTAB reads from 'source' until end-of-file or the 'end' section
is encountered.
MACTAB then collects individually compiled
sections, and writes them to the file 'r-file'.
Any error will abort production of the r-file,
but will continue scan of source.
.sp 3
.NH 1
SOURCE FORMAT.
.PP
The r-file source description is made up
of several distinct parts, two of which are optional.
These parts are called SECTIONS.
The order of the sections is not critical, but
some sections require that other sections be
previously defined.
It is suggested that the user follow the
given section ordering and internal arrangement.
.PP
A section has the general form:-
.sp
            section-name
.br
            .
.br
            .
.br
            <description>
.br
            .
.br
            .
.br
            %
.sp 2
.PP
The '%' at the end indicates end-of-section.
The section-names and end-of-section
character must begin in column one of the source line.
.bp
.NH 1
HEADER SECTION.
.PP
The header section generally describes the
target machine's architecture to MAC.
This section produces the header record
at the beginning of each r-file.
.PP
MACTAB allows the definition of special
pseudo opcodes for the definition of
constants during the assembly process.
These opcodes, called Define Constant (dc)
opcodes, can be given one extra identifying
character, appended to the 'dc'.
.IP "      e.g.-" 16
dc4     dcf     dc.
.br
dcb     dch     dc@
.PP
The identifying character must be a legal
alphanumeric, so that the whole dc opcode
forms a legal MAC label.
.sp 2
.PP
Each dc can be constructed to define
constants of particular length and/or format.
Thus - 'dcb' may be declared to define one byte
of storage with a constant in it, 'dch'
to define a half-word constant, 'dcw'
to define a full-word constant.
The source line required is:-
.IP "" 12
dc      <character>    <format>
.PP
The required <format> field is described in detail
later in this manual. Please refer to the
FORMATS section for this description.
.sp 2
.PP
MACTAB and MAC allow the user to define
a default dc, which has no identifying
character. The source line required is:-
.IP "" 12
defmt   <format>
.PP
Where <format> is as described later.
.sp 2
.PP
Program counter incrementation is important
for addressing relative to the program counter.
The pc can be pre-incremented (before an instruction
is executed) or post-incremented (after an instruction is executed).
Declaration is as follows:-
.IP "" 12
pc      post           OR
.br
pc      pre
.bp
.PP
The assembler requires the width of the basic
address unit (byte) in bits. Declaration is:-
.IP "" 12
byte    <width>
.PP
Where <width> is a constant (numeric)
with a value equal to the byte width.
.sp
.PP
Similarly - the assembler needs to know the
number of bytes per word.
It's definition is:-
.IP "" 12
word    <length>
.PP
Where <length> is the number of bytes per word.
.sp 2
.PP
The user is requested to supply some opcode value
that is treated as illegal by the target machine.
If none is supplied - zero (0) is assumed.
The declaration is:-
.IP "" 12
ii      <value>
.sp 2
.PP
MAC allows the user to supply a string of up
to thirty characters that is printed at the head of each
page generated by MAC (listings and so on).
This is optional.
.IP "" 12
mac     "Title string"
.sp 2
.PP
MAC allows the user to give a page length in lines
for listings and so on. This is specified as a number of lines.
The default is 60.
.IP "" 12
page    <lines>
.bp
.PP
A typical declaration is illustrated below.
The format descriptors on the define constant
lines are described later.
.IP "" 12
.sp
header
.br
pc      post
.br
byte    8
.br
word    2
.br
dc      b       a:8
.br
dc      f       a:16
.br
defmt           a:8
.br
mac     "Dummy machine"
.br
page    60
.br
ii      0xff
.br
%
.bp
.NH 1
LITERALS SECTION.
.PP
The literals section allows definition of the
special class of labels called literals.
Literals are standard MAC labels that
have been defined as reserved,
and have no value assigned to them.
Literals cannot be used in expressions,
but are useful in argument pictures
for recognising different pictures.
A literal cannot be redefined as a label with
a value.
A typical definition may be:-
.IP "" 12
.br
literals
.br
x
.br
y
.br
a
.br
%
.sp
.PP
The literals section (if needed) MUST appear
before the args section.
This section is optional and may be totally
omitted if desired.
.sp 3
.NH 1
LABELS SECTION.
.PP
This is the only other optional section.
This section allows the user to pre-define
up to sixteen labels and assign them values.
These labels will always be defined to
all users of the r-file,
and so provide a mechanism
for remembering frequently used
addresses or values, such as
subroutines in the monitor Read-Only-Memory.
The labels are defined to have non-relocatable
values.
This section may appear anywhere in the source
description.
The definition is:-
.IP "" 12
label    <value>
.PP
Where <value> is a legal MAC constant.
A typical section definition is:-
.IP "" 12
.sp
labels
.br
adc     4
.br
nul     0
.br
tty     0xc0fe
.br
mask    0777
.br
%
.bp
.NH 1
FORMATS SECTION.
.PP
This section describes the format of the binary instruction
(or data for a define constant) to MAC.
Generally - each class of instructions (zero
page, pc relative, register indexed etc)
will have a seperate format descriptor.
Each descriptor can define one instruction
or data format.
.PP
Each descriptor is made up of field descriptors,
called subset descriptors.
Each subset is defined as follows:-
.IP "" 12
<subset-name>:<subset-width>
.sp
.PP
Each subset name is one identifying character.
The letter 'o' means opcode value, '!' means
the current value of the program counter,
a '#' means a constant will follow,
and the letters 'a' to 'm' mean the values of argument
expressions one to thirteen respectivly.
.PP
Pre-fixing the subset names 'a' to 'm' with the
letter 'p' indicates that this argument should
be assembled program counter relative.
That is - the value assembled is the value of the
expression minus the current value of the location
counter.
Pre-fixing the subset names 'a' to 'm' with the
special field 'r<n>' where <n> is a decimal constant,
implies that the argument expression should be
assembled with the most significant <n> (number) of bits
in the expressions value
swapped with the least significant <n> bits.
This format prefix assumes that the maximum
length of the expression's value will be
two times the constant <n>.
.PP
The name 'o' implies assemble the value
of the current opcode being assembled here.
Then 'o:8' implies that the opcode
value should be assembled in the next 8 bits
of memory.
Thus 'a:16' means assemble argument 'a' (argument one)
in the next 16 bits, '#123:16' means assemble
the decimal constant 123 in the next 16
bits, 'pa:8' means argument one made pc
relative in the next 8
bits, 'r8a:16' means assemble argument one
in 16 bits, with the first 8 bits and the next 8 bits
swapped.
.PP
The full descriptor is made up of several
subset descriptors grouped together.
.IP "" 12
.sp
|------------------------------|
.br
| opcode | arg 1 |    arg 2    |
.br
|------------------------------|
.br
    8        8         16            (bits)
.sp 2
The above instruction may be described as:-
.sp
o:8a:8b:16
.bp
.PP
Each descriptor must be prefixed with a number
indicating the number of arguments to this instruction,
blanks/tabs, then the format descriptor.
.IP "      e.g.-  " 12
2       o:8a:8b:16
.sp
.PP
A typical definition may be:-
.IP "" 12
.br
formats
.br
0       o:8
.br
2       o:8b:4a:12
.br
1       #0xf:4o:4a:8
.br
1       o:6a:12
.br
1       a:4o:4#0x12:8
.br
1       o:8r4a:8
.br
1       o:8pa:8
.br
%
.sp 2
.PP
MACTAB scans each format descriptor for validity,
and reports and inconsistencies.
The total width of the format descriptor in bits
must be an even multiple of the width of the
basic address unit (byte). For this
reason, the header section must be defined before
the formats section.
.PP
Each descriptor is assigned a number,
starting at zero (0) and being incremented by one
for each new descriptor.
These logical numbers are the only method of referring
to a format descriptor.
.bp
.NH 1
OPCODES SECTION.
.PP
This section is used to describe all
of the possible opcode symbolics (except the pseudo
opcodes which are always defined) and the
values of each symbolic.
The MAJOR CLASS of opcodes must first be defined.
This is the maximum number of different
values any opcode symbolic may have.
Each different argument picture may be used to
select a new opcode class (a particular value
out of a class of values).
Thus - the construct
.sp
.DS
		'sub   expression'
.DE
.sp
.PP
may select a value from
a list for the 'sub' instruction, while
the construct
.sp
.DS
		'sub   #expression'
.DE
.sp
.PP
may be set
to select another value from the same list
for the 'sub' instruction.
The table may be laid out in columns, each column
corresponding to a class, and each row corresponding
to an opcode symbolic.
.IP "" 8
.br
opcodes
.br
class  <n>
.br
add    1     value<0>  value<1>  .  .  .  value<n-1>
.br
sub    1     value<0>  value<1>  .  .  .  value<n-1>
.br
jmp    2     value<0>  value<1>  .  .  .  value<n-1>
.br
%
.sp
.PP
This example may have class<0> values
defined for zero page addressing, class<1>
for immediate mode, and so on.
A logical connection can be made between
columns and argument pictures.
The normal practice is to allow one column
for each type of argument picture.
The single number adjacent to the
opcode symbolics is the number
of a format descriptor to use for
this instruction if no other is selected.
(See args section for the method of selecting
classes of opcode values and format descriptors
depending on the argument pictures.)
.bp
.PP
If a class of opcode values (a particular
argument format) is desired to be made
illegal for an opcode symbolic, the user must
make the opcode value for that symbolic and class equal
to the selected illegal instruction value defined
in the header section. Zero is the default.
If MAC is requested to assemble an instruction
with an illegal opcode value, it will reject the source line
and terminate in error.
As an example, the 'sub' symbolic may be allowed
to have class 1, 2 and 4 argument pictures,
but not classes 0, 3 and 5.
The declaration may be:-
.IP "" 8
sub   1     0  0x10  0x20  0  0x40  0
.sp
.PP
Up to 256 different opcode symbolics may be defined,
with no limit on the maximum class size.
.PP
To make this section a little more understandable,
The reader is referred to the two appendicies,
wherein he will find r-file descriptions for
the Motorola 6800 and the 6502 micro-computers.
Careful study of these should lead to a better
understanding of the opcode section.
.bp
.NH 1
ARGS SECTION.
.PP
This section is the most important section of the
six sections of the table formatter.
This section takes the user's description of
the argument pictures and using a top-down
recursive descent algorithm builds a
finite-state parser table for the cross assembler.
Each picture is made up of literals, the reserved
keyword 'expr' meaning an expression, delimiters (commas
and so on) and some required characters,
such as a '#' meaning immediate mode, '$' meaning
zero-page addressing, '@' meaning indexed.
These characters are 'required' in the sense that
they must be present in the argument picture for
MAC to recognise that format of picture.
.PP
MACTAB recognises the keyword 'expr', and
where-ever it occurs, MACTAB substitutes a call
to an expression parser, that is pre-defined.
.PP
The user can associate a series of actions to
perform upon recognition of an argument picture.
The actions currently implemented are:-
.IP "    1)" 8
Select a new format descriptor
.IP "    2)" 8
Select a new class of opcode values for
this symbolic

.PP
Typical argument pictures could be:-
.IP "" 12
.br
expr
.br
expr , x
.br
expr , ( y )
.br
# expr
.br
a , expr
.br
( $ expr ) , y
.sp
.PP
Selection of actions is by addition of
four constants after the picture but on the same
source line.
The first describes which action to take (if any).
Each bit in the constant means a particular argument.
If a bit is set, MAC will try to perform the requested
action.
The remaining three constants are arguments to
the actions, specifying such things as
which new format descriptor, and which
new opcode class.
If the bits are numbered 0 as the least-significant bit
(right-most), then if bit zero is set, MAC will select
a new format descriptor, and assume the fourth
argument constant will be the new format descriptor.
If bit one is set, MAC will select a new class of
opcode values, that class being the value of
the third constant.
Selection actions are optional.
The actions are selected by preceeding the four constants
with a brace '{'.
All four constants are needed after the brace.
Constants should be seperated by blanks and/or tabs.
.bp
.PP
Several pictures should always be defined by
the user.
The picture " " (a string) should be defined,
for the dc and title pseudo opcodes.
A blank line is equivalent to the case of
an instruction having no argument picture.
.PP
The special case of a label in an argument picture may appear,
as this indicates that a label (not an expression) is
required here.
See appendix 1 for examples of real definitions.
.sp 5
.NH 1
END SECTION.
.PP
The end section is needed to actually
create the named r-file.
Upon recognition of this section,
MACTAB collects the compiled sections,
and writes them in their correct order
onto the r-file.
Any errors except the end section
missing will cause the r-file not
to be produced.
If an end-of-file is encountered before
the end section,
MACTAB reports this as a warning only,
and assumes an end section.
In this case only, an r-file will
be produced.
No terminating '%' is required for this section.
.bp
