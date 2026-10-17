.na
.TL
SUMacC: Stanford UNIX Mac C Development Kit
.AU
Bill Croft
.AI
Stanford University
Medical Center
SUMEX Project, rm TB105
Stanford, CA  94305

Version 2.0, 11/84
.AB
This note describes SUMacC, 
a UNIX cross development environment 
for the Apple Macintosh.
.AE
.NH
Introduction
.PP
This note describes SUMacC (pronounced like 
.I "poison sumac" 
), a cross development environment that runs
on UNIX which is used to compile and download C programs for execution on the
Apple Macintosh.
.PP
SUMacC was released originally in June 84 in a beta test form.  The current
release includes all the bug fixes since then, plus additional
material that has been collected thru the 
.I info-mac
bulletin board on SUMEX.
It includes most of the updates released by Apple in the 'Mac Supplements'
of July 20 and October 22.
The 'Imagewriter pre-release' of August 1 (for wide platen Imagewriters)
is not yet integrated.
.PP
We are distributing this package under the condition that it may
be 'used' but not 'sold' without our permission.
The package is under a Stanford copyright
which must be retained on all copies of this software.
Any fixes or enhancements should be reported
back to us, for incorporation into future releases.
While we will attempt to fix bugs and provide new features, no
warrantee is expressed or implied.  You are basically on your own.
.NH
Prerequisites
.PP
The package is currently only setup for a VAX running Berkeley
UNIX or Eunice.  It should be convertable to any other UNIX box having
a 68000 C compiler.  For a list of sites that have or are porting
SUMacC to other systems, look at the file 'sites' and 'etc/port'.
.PP
We assume you have some release of the Apple 
.I MacTerminal
program.
This is used to download files from UNIX
to your Mac disk.  Your RS232 connection from UNIX to the Mac must
pass all 8 bits and not impose any 'automatic' flow control;  i.e.,
connections thru terminal switchers or networks may not work.
.PP
It's also handy (but not necessary) to have at least one Lisa system
available, since this is how new software is
released from Apple.
.NH
Contents of the Kit
.PP
The following directories/files are in the distribution:
.IP sumacc.ms 12
This file, in \-ms macro format.
.IP Makefile
Master makefile that calls Makefile's in subdirectories.
.IP cc42
C compiler binaries for 4.2 BSD.
.IP cc41
C compiler binaries for 4.1 BSD.
.IP cceun
C compiler binaries for VMS Eunice.
.IP h
Macintosh header files, copied to /usr/include/mac.
.IP lib
Macintosh library files.
.IP cmd
Resource maker and other related commands.
.IP mac
Mac C example and test programs
.IP man
Manual pages.
.IP ws
Reference copy of most of the sources distributed with the Lisa Workshop.
.NH
Installation
.PP
If you are running VMS Eunice, first examine the README and other 
files in the cceun directory.
.PP
The makefiles included have been conditionalized so that you can
change where the files reside after installation.  
At Stanford for historical reasons
the binaries are in
.I /usr/stanford/bin,
the library stuff goes in
.I /usr/sun/lib,
and the include files are kept in
.I /usr/include/mac.
If you change where things are located, you will have to: (1) edit
the master Makefile and/or the sub-Makefiles to change the installation
directory names.  (2) Edit 
.I cc*/cc68.c, 
the C compiler driver so that
it can find the assembler/loader/etc.  
This is easy to change for
.I /usr/stanford/bin,
but since the 
.I ld68 
and 
.I as68 
binaries provided have
.I /usr/sun/lib
wired in, you may want to leave that one alone.
Use
.I mkdir
to make these directories if they don't already exist.
Another possibility to change the file locations is to use the
symbolic link facility on 4.2 to (e.g.) 
.I
link -s /usr/local /usr/stanford/bin.
.R
.PP
This release contains a compiler with a larger symbol table, so
your should
.I cd
to 
.I cc42
or
.I cc41
or
.I cceun
and type
.I make.
This will install the compiler.
If you have compiler sources, the only change made in this release was in file
ccom/manifest;  the define for SYMTSZ was increased from 800 to 1600.
.PP
.I "4.1 BSD or Eunice:"
If you are running 4.1 (or a Eunice based on 4.1), edit 
lib/Makefile so that the lint library is constructed with 'mk41lint';
(4.1 lint doesnt understand the '\-C' switch.)
.PP
Now from the main directory, do a
.I make.
This will compile and install the commands, Mac library, 
Mac lint library,
include files, manual pages, and example programs.
.PP
Find your disk with MacTerminal on it.
Locally, we use a disk that also contains
.I 
Resource Mover, Set File, Examine File, 
.R
and
.I MacsBug
utilities.  These are useful utilities supplied on the
.I MacStuff
disks that come with the Workshop supplement.
You should now be able to follow the instructions under
.B Downloading
below to try out some of the programs in the UNIX
.I mac
directory.
.NH
Example Programs
.PP
The C example programs provided are:
.IP macscrawl 3
A primitive text/drawing program.
Its commands are
single keyboard characters;  examine the source code before
trying to use it.  Try the 'm'agnifying lens to zoom in and out
on sections of the screen and/or the lens itself.
.IP grow
A straight translation of the Pascal Grow program provided
in the Workshop.  Most toolbox managers
are exercised.  
You might runoff a listing of grow.c and grow.p
to compare how the Pascal was translated to C.
.I
Note a bug: 
.R
both the Pascal
and the C version apparently do not save TextEdit state when the
Notepad desk accessory is in use, resulting in weird behavior
with that accessory.
.IP insane
This tests the SANE IEEE 80 bit floating point package and
numeric conversion.  Floating operations seem to average
about 1 millisecond (well you can't say it isn't accurate...).
.IP "alert   "
Tests alert and dialog boxes, from Bill Schilit.
.IP calendar
A calendar desk accessory, from Mike Schuster.
.IP ramdisk
A ramdisk device driver, converted to C from workshop assembler.
.IP hang
A hangman game, from Dan Winkler and Ken Arnold.
.IP kermit
A preliminary implementation of kermit, from Steve Engle.  'cmd/kermit.c'
is the UNIX side.  
.IP mfinder
A 'mini' finder that can start a program several times faster than the
real finder.
.IP print
A 'print' desk accessory that can print several files in the backround,
from Mike Schuster.
.NH
Typical Compilation Cycle
.PP
Examine the Makefile and *.rc (resource compiler input)
files in the mac directory.
The makefile typically looks something like:
.DS
.I
cc68 \-m grow.c
rmaker grow.rc
.R
.DE
.PP
Cc68 compiles, assembles, and loads the C program onto a file
called
.I b.out.
Then the resource maker (sometimes called the resource compiler)
reads the file grow.rc.
This file directs rmaker to read the binary b.out and combine it
with some hex icon information in grow.rc.  The resulting
output, which would be directly executable on the Mac, is
left in the file grow.rsrc.  (rsrc stands for 'resource', and
is the same suffix convention that is used on the Lisa).
.PP
From time to time, you will also want to 
.I lint
your sources to ensure proper matching of argument and data
types.  For example:
.I
lint grow.c -lmac
.R
.NH
Downloading
.PP
Macget and macput on UNIX (from Dave Johnson, Brown U) are used to up and
download files with MacTerminal.  See the manual
pages for macget and macput for full details;
there is also a protocol description manual in 'cmd/macput.doc'.  
Typical usage is very
simple;  for example, to download the 'grow.rsrc' program
produced above:  (1) run MacTerminal on the Mac.  (2) through
MacTerminal, type the UNIX command:
.I
macput -r grow.
.R
(3) macput will now take over and perform the file transfer, in
cooperation with MacTerminal.  When the transfer is complete you
can:  (4) quit MacTerminal and run your program.
.PP
If your program uses a special icon (as do the samples provided),
a strange feature of the 'finder' requires that the 'bundle'
bit be set at least once on one of these binaries.  You
can use the 'Set File' program for this.  After finder
caches the icon in it's 'DeskTop', further bundle bits are
not needed.
.PP
Although macput / MacTerminal is the simplest way to download
files, a 
.I kermit
also exists for the Mac.  You may find this especially useful
if you need to perform file transfers with other hosts besides UNIX.
See '{cmd,man,mac}/kermit*'.
.KS
.NH
Toolbox Programming
.NH 2
Argument Passing
.PP
When using
.I
Inside Macintosh
.R
to program in C, you must do a mental translation of the
Pascal calling conventions into corresponding C conventions.
.KE
They are quite straightforward and even if you goof, lint will warn you.  
Take a look at the sample programs to get the hang of it.
Remember that a Pascal VAR, means a variable passed by
reference (address).
Here are the conventions:
.PP
When passing a structure or string, pass its address.
Pointers or handles are passed the same as in C (by value).
If Pascal expects a VAR, pass its address (if it's 
a structure or string, you've already 
converted that to an address (rule above)).
.PP
SUMacC also does automatic translation for 'integer'
and 'string' formats during Toolbox calls.  The default
integer in Pascal is a short, while C assumes it is a long.
Pascal strings are not null terminated but instead begin
with a byte count.
Some further implications of this mechanism for strings
are discussed in a section below.
.NH 2
Handle Dereferencing
.DS
The Pascal notation
.I
hTE^^.viewRect := pRect;
.R
is translated as
.I
(*hTE)\->viewRect = pRect;
.R
.DE
.NH 2
Pascal Bird-Nest Soup
.PP
Compare mac/grow.c with mac/grow.p.  Notice how the C version
can use many less nesting levels than Pascal by proper use of
.I
break, continue, 
.R
and
.I return
statements.  This greatly improves the readability
and understanding of C programs.
C programs also allow much easier setup of initialized global
data and strings.  Part of the reason 'resources' are so
common in the Pascal example programs, is that they allow
an 'easy' way for Pascal programs to access initialized data.
Of course this must be tempered with the fact that these resources
must be named and numbered in another file and introduce another
level of indirection in understanding a program listing.
The plus side to resources is that they allow easy language
translation of your program, but how many of the programs you
write will have international distribution?  If source is available
you could achieve a similar affect by placing your language
dependent strings in an ifdef'ed section or include file.
.NH 2
String Utilities
.PP
Normally on Toolbox calls, the routine
.I mactrap.s
converts automatically
between the C string convention
and the Pascal strings expected.  This occurs both
on passing the argument via the function call, and on string
VARs returned.
Thus in 99% of the cases you need do nothing special for strings.
However occasionally a Pascal string is embedded in a Toolbox
structure or something similar.
In such cases there are these functions to help out (see mac/grow.c
for an example call and lib/strconv.s for more detailed info):
.IP c2pstr 8
This converts a C to Pascal string in-place by shifting the C string
down one position and inserting a byte count at the beginning.
.IP p2cstr
This is the converse operation of converting a Pascal to a C string;
the byte count is removed and a trailing zero byte added.
.IP isapstr
This is a function;  it returns the argument, which is
a string pointer, negated.  If passed to a toolbox routine that expects
a string, this negative pointer will override the default
C to Pascal string conversion.  In effect the function states 'this
argument is-a Pascal string'.
.NH 2
Heap vs. C Bss
.PP
Uninitialized globals in C are placed in an area called 'bss'.
In this implementation, bss is reserved by appending
that amount of trailing zero bytes to the CODE resource.
(The Mac segment loader has no convenient mapping for
a bss notion contiguous to text/data;  sorry).
Thus you can use uninitialized globals as normal, but if you
have any truly large structures (over a few hundred bytes), you
should call the Toolbox routines 
.I NewPtr
or
.I NewHandle
to get a piece of the heap,
and place the pointer or handle to that area in a global variable
instead.  This is actually a common C coding practice on
UNIX, using the 
.I malloc
routine there.
.NH 2
Pascal Toolbox Calling on C Routines
.PP
Yes, this does happen occasionally.  For an example see mac/grow.c in the
routines ScrollUp, ScrollDown.  Your C function must
call an assembly helper 
.I getpargs.
.PP
This function is called with one argument, a pointer to a 'local'
structure to receive the Pascal arguments.  Your structure declaration
must include each Pascal argument, in 'stack' order, and with
the proper 'type' associated with each Pascal variable.
See the Inside Mac section 'Programming in Assembly Language', 'Calling 
conventions, stack-based routines'.  You may also
want to look at lib/getpargs.s.
.PP
Getpargs 'pops' the Pascal arguments off the stack and into your
local structure.  As a side effect, it also returns as a value,
a pointer to the Pascal return value (on the stack).  So you
can set this before returning, if your C routine is called as a function.
.PP
Below is an example C routine that is 'declared' in Pascal as
.I
FUNCTION hisproc (one: Ptr; two: INTEGER) : INTEGER;
.R
.DS L
	hisproc()
	{
		struct {
			short	two;
			Ptr	one;
		} pargs;
		short	*pret;

		pret = (short *) getpargs(&pargs, sizeof pargs);  /* get arguments */
		if (pargs.two != 0) ...  /* use the 2nd argument */
		*pret = returnvalue;
	}
.DE
.NH 2
Floating Point
.PP
See mac/insane.c for examples of SANE usage.  However you may
be better off with the fixed point arithmetic provided by
.I
FixRatio, FixMul, FixRound 
.R
discussed in the Toolbox Utilities
section of Inside Mac.
.PP
The MIT C compiler we are using also has its own built-in
floating point formats and library routines (in libc.a),
but we have no experience with these.
They are certainly not the IEEE standard.
.NH 2
Naming conventions.
.PP
Since Pascal is case insensitive, some of the names present in the
original Pascal header files had inconsistent case conventions.
Apple realized the headaches this could cause for C programmers,
and in October 84 released a small set of rules that govern
case in names.  When you invent new names, you should use these
rules.  
.IP "1." 4
The first character of a name is upper case if the symbol is:
a toolbox routine name,
a type,
an assembler global symbol.
.br
e.g.: InitGraf, MoveTo, ProcPtr, MonkeyLives
.IP "2."
The first character of a name is lower case if the symbol is:
a field,
a constant,
a variable.
.br
e.g.: shiftKey, everyEvent, fInvisible
.IP "3."
If the name consists of multiple words, the subwords are capitalized.
.IP "4."
If there are acronyms within the name, they are cased the same.
.br
e.g.: fCTS, ctsHold, TEHandle, TENew, eofErr
.NH 2
Low memory globals
.PP
The Pascal workshop defines several functions whose only purpose is to
get or set certain low memory assembler globals.  For example
TEGetScrapLen returns the contents of assembler global TEScrpLength;
TESetScrapLen can be used to set this global.  In C it is simpler
to just setup a define such as: '#define TEScrapLen (*(short *)0xab0)'.
Then the 'variable' TEScrapLen can be used as normal in the left or
right hand side of any expression.
.PP
This has been done for all of the get/set functions provided in the
workshop.  Below is a list of these locations.  If you have others
it's easy to add them to your own program.
DoubleTime CaretTime VBLQHdr EvQHdr DrvQHdr FSQHdr VCBQHdr SysPPtr 
DefVCBPtr DlgFont TEScrapHandle TEScrapLen AlertStage.
.NH 
Compiler Sources
.PP
The distribution contains only sources for the code
written at here at SUMEX (along with code contributed by
other groups).  Since the compiler/loader is based on
the Bell Labs (Johnson) Portable C Compiler, we must be careful
about distributing copies of this.  
It's also unclear what good the compiler
will do for you, since it's a large and somewhat unwieldy
amount of code.
As an interim policy we
will make these sources available to other Universities if there
is enough interest.
However don't ask unless you plan some active
development in this area.
.NH
Currently Unimplemented
.PP
SUMacC currently does not support overlays, so on a small Mac you are
limited to about 80K bytes of program.
There are no restrictions on the
size of SUMacC programs, other than the available physical memory.
The Apple workshop Pascal compiler generates PC-relative code, so its segments
are limited to 32K bytes each.  
SUMacC dynamically relocates the program
at run-time, by following relocation chains run-length encoded though
the 'unused' high order byte of long word addresses.
.PP
The Graf3D package is not converted (but see
lib/TODO for a hint on how to do it).
Some of the more esoteric rmaker types are still unimplemented.
C library and header files will always be somewhat behind in tracking
the Pascal versions
since Apple only
releases code in Pascal.  It's handy to have at least one Lisa around so
you can, for example, translate a new header file into C on your
own if you need it desparately.
.NH
Acknowledgements
.PP
The 68000 C compiler used by SUMacC is the Bell Labs (Johnson) 
portable C compiler, ported by Chris Terman of MIT and used in
the MIT NU project and the Stanford SUN system.  This same compiler /
assembler / loader is used in many of the 68K UNIX boxes currently
on the market.  The assembler is by Mike Patrick, also of MIT.
Many modifications and bug fixes have been applied by folks at
Stanford, MIT, and Lucasfilm:  Jeff Mogul, Bill Nowicki, John Seamons,
Vaughan Pratt, Eric Ostrom, and a cast of thousands.
.PP
Dave Johnson of Brown Univ. contributed the excellent macget/macput
programs that make downloading painless.  Among the many people
contributing improvements, new example programs, and bug fixes
are:  Mike Schuster of CALTECH, Dan Winkler and Steve Engle
of Harvard, Bill Schilit
at Columbia, Joe Pallas and  Steve Gross at Stanford, John Seamons
at Lucasfilm, John Peterson at Utah, Ben Hyde of Intermetrics,
Bruce Horn of Apple/Adobe, Van Jacobson at LBL, and many others.
Thank you all!
.NH
Freeware
.PP
If you think sharing is a good idea, then I
encourage you to make your own projects available to the
info-mac community.  Post a note to info-mac and tell folks
how to FTP the program from your site;  alternately SUMEX
would be glad to archive your programs for FTPing in our
info-mac directory.
