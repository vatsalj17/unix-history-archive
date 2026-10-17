|	insane.s
|
|	Pascal to assembler conversion of dec2str/str2dec (sanescan).
|
|	This is an insane way to do this.  Much better would be to have
|	sanescan part of the fp68k or elems "package" so that us C programmers
|	could call it via simple traps.   --bc
|
|
| {^ne 18 }
|         {-----------------------------------------------------------------
|         ** conversions between the numeric types and strings.
|         ** { (these conversions have a built-in scanner/parser to convert
|         ** between the intermediate decimal type and a string.)
|         -----------------------------------------------------------------}
|
|         {$i mc/sanescan.text}
| {*
| ** 01sep82: written by jerome coonen.
| ** 18jan83: infs as ++++/---- and nans with hexits. (jtc)
| *}
|
| {*
| ** format an input decimal record, according to input format
| ** structure, and write to string.  see sane pascal interface
| ** document for an explanation of the data types.
| *}
| procedure dec2str(var df: decform; var d: decimal; var s: decstr);
|
| var
|     i      : integer;
|     ds, es : decstr;
|
|
|
|     {*
|     ** convert an integer to a signed decimal string, for printing
|     ** exponents.
|     *}
|     procedure int2str(ie: integer; var is: decstr);
|     var
|         i: integer;
|         s1 : string[1];
|
|     begin
|
		.text
int2str:
|
|
         tstw   a7@(-4182)       | /efaa
         link    a6,#-86         | /ffaa
         movl    d7,a7@-
|         i := ie;    {make integer local}
         movw    a6@(16),d7       | /0010
|         if i < 0 then
         tstw   d7
         bges   .+8             | /0008
|             i := -i;
         movl    d7,d0
         negw   d0
         movw    d0,d7
|         s1 := '1';
         movw    #305,a6@(-4)     | /0131, /fffc
|
|         s1[1] := chr(ord('0') + (i mod 10));
         moveq   #1,d0           | /01
         movl    d7,d1
         extl   d1
         divs    #10,d1          | /000a
         swap    d1
         tstw   d1
         addw   #48,d1          | /0030
         .word	/4e71,/4e71
         movb    d1,a6@(-4,d0:w) | /fc
|         is := s1;
         movl    a6@(12),a0       | /000c
         movw    a6@(-4),a0@     | /fffc
|         i := i div 10;
         movl    d7,d0
         extl   d0
         divs    #10,d0          | /000a
         movw    d0,d7
|         while i <> 0 do
         tstw   d7
         beqs   .+56            | /0038
|         begin
|             s1[1] := chr(ord('0') + (i mod 10));
         moveq   #1,d0           | /01
         movl    d7,d1
         extl   d1
         divs    #10,d1          | /000a
         swap    d1
         tstw   d1
         addw   #48,d1          | /0030
         .word	/4e71,/4e71
         movb    d1,a6@(-4,d0:w) | /fc
|             insert(s1, is, 1);
         pea     a6@(-4)          | /fffc
         movl    a6@(12),a7@-    | /000c
         movw    #1,a7@-        | /0001
         bsr     _ins
|             i := i div 10
         movl    d7,d0
         extl   d0
         divs    #10,d0          | /000a
         movw    d0,d7
         bras   .-56            | /0038
|         end;
|
|         if ie < 0 then is := concat('-', is)
         tstw   a6@(16)          | /0010
         bges   .+44            | /002c
         movl    a6@(12),a0       | /000c
         movl    a0,a7@-
         pea pc@(58-2)            | /003a
         movl    a6@(12),a7@-    | /000c
         pea     a6@(-86)         | /ffaa
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-86),a1      | /ffaa
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         movl    a7@+,d7
         unlk    a6
         movl    a7@+,a0
         addw  #10,a7          | /000a
         jmp     a0@
|
         .word   /c94e,/5432,/5354,/5220  | ".nt2str "
         .word   /0002,/012d              | "...-"
|     end;
|
|
|
| begin  { dec2str }
|     {*
|     ** cases 0, infinity, and nan are treated separately.
|     ** in any case, a lead sign or space is printed.
|     *}
|
		.text
		.globl	_dec2str
_dec2str:
|
|
         tstw   a7@(-5462)       | /eaaa
         link    a6,#-1370       | /faa6
         moveml #/118,a7@-
         movl    a6@(12),a4       | /000c
         movl    a6@(16),a3       | /0010
|     if dsgn <> 0 then
         tstb   a4@
         beqs   .+12            | /000c
|         s := '-'
         movl    a6@(8),a0        | /0008
         movw    #301,a0@       | /012d
         bras   .+10            | /000a
|     else
|         s := ' ';
         movl    a6@(8),a0        | /0008
         movw    #288,a0@       | /0120
|
|     {*
|     ** if it is a nonzero, finite number, place a decimal
|     ** point after the lead digit and print the exponent field.
|     ** exponent must be adjusted to align point after lead digit.
|     ** the special cases are handled trivially.
|     *}
|
|     case dsig[1] of
         moveq   #1,d0           | /01
         movb    a4@(4,d0:w),d1   | /04
         andw  #255,d1         | /00ff
         subw  #48,d1          | /0030
         beqs   .+24            | /0018
         subw  #15,d1          | /000f
         beq     .+352           | /0160
         subw  #10,d1          | /000a
         beqs   .+56            | /0038
         subqw  #5,d1           | /5
         beqs   .+86            | /0056
         bra     .+384           | /0180
|
|     '0':
|         s := concat(s, '0.0');
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea pc@(984-2)           | /03d8
         pea     a6@(-250)        | /ff06
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-250),a1     | /ff06
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         bra     .+888           | /0378
|
|     'i':
|         if dsgn <> 0 then
         tstb   a4@
         beqs   .+16            | /0010
|             s := '----'
         movl    a6@(8),a0        | /0008
         lea pc@(934-2),a1        | /03a6
         movl    a1@+,a0@+
         movw    a1@,a0@
         bras   .+14            | /000e
|         else
|             s := '++++';
         movl    a6@(8),a0        | /0008
         lea pc@(914-2),a1        | /0392
         movl    a1@+,a0@+
         movw    a1@,a0@
         bra     .+854           | /0356
|
|     'n':
|         {*
|         ** insert the hexits from dsig and insert a colon
|         ** after the leading four.
|         *}
|         begin
|             { dsig has form nzzz...z with up to 16 hexits }
|             s := concat(s, 'nan''', copy(dsig, 2, (length(d.sig) - 1)), '''');
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea pc@(886-2)           | /0376
         movl    a0,a7@-
         pea     a4@(4)           | /0004
         movw    #2,a7@-        | /0002
         movb    a4@(4),d0        | /0004
         andw  #255,d0         | /00ff
         subqw  #1,d0           | /1
         movw    d0,a7@-
         pea     a6@(-272)        | /fef0
         bsr     _copy
         movl    a7@+,a0
         pea     a6@(-272)        | /fef0
         pea pc@(844-2)           | /034c
         pea     a6@(-378)        | /fe86
         movw    #4,a7@-        | /0004
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-378),a1     | /fe86
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
|             { s has form *nan'zzz...z' where * is - or blank }
|
|             { delete trailing 0's, up to lead 9 chars: *nan'zzzz }
|             i := length(s) - 1;  { ignore the ' ending s }
         movl    a6@(8),a0        | /0008
         movb    a0@,d0
         andw  #255,d0         | /00ff
         subqw  #1,d0           | /1
         movw    d0,d7
|             while (s[i] = '0') and (i > 9) do
         movl    a6@(8),a0        | /0008
         .word	/4e71,/4e71
         movb    a0@(0,d7:w),d0   | /00
         andw  #255,d0         | /00ff
         cmpw  #48,d0          | /0030
         seq     d0
         cmpw  #9,d7           | /0009
         sgt     d1
         andb   d1,d0
         beqs   .+6             | /0006
|                 i := i - 1;
         subqw  #1,d7           | /1
         bras   .-34            | /0022
|             delete(s, (i + 1), (length(s) - i - 1));
         movl    a6@(8),a7@-     | /0008
         movl    d7,d0
         addqw  #1,d0           | /1
         movw    d0,a7@-
         movl    a6@(8),a0        | /0008
         movb    a0@,d0
         andw  #255,d0         | /00ff
         subw   d7,d0
         subqw  #1,d0           | /1
         movw    d0,a7@-
         bsr     _del
|
|             { addw a : if any more than 4 hexits left: *nan'zzzz???'  }
|             if length(s) > 10 then
         movl    a6@(8),a0        | /0008
         movb    a0@,d0
         andw  #255,d0         | /00ff
         cmpw  #10,d0          | /000a
         bles   .+18            | /0012
|                 insert(':', s, 10);
         pea pc@(714-2)           | /02ca
         movl    a6@(8),a7@-     | /0008
         movw    #10,a7@-       | /000a
         bsr     _ins
|
|             { delete up to 3 leading 0's }
|             i := 6;
         moveq   #6,d7           | /06
|             while (s[i] = '0') and (i < 9) do
         movl    a6@(8),a0        | /0008
         .word	/4e71,/4e71
         movb    a0@(0,d7:w),d0   | /00
         andw  #255,d0         | /00ff
         cmpw  #48,d0          | /0030
         seq     d0
         cmpw  #9,d7           | /0009
         slt     d1
         andb   d1,d0
         beqs   .+6             | /0006
|                 i := i + 1;
         addqw  #1,d7           | /1
         bras   .-34            | /0022
|             delete(s, 6, (i - 6))
|         end;
         movl    a6@(8),a7@-     | /0008
         movw    #6,a7@-        | /0006
         movl    d7,d0
         subqw  #6,d0           | /6
         movw    d0,a7@-
         bsr     _del
         bra     .+600           | /0258
|
|     '?':
|         s := concat(s, '???.???');
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea pc@(620-2)           | /026c
         pea     a6@(-466)        | /fe2e
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-466),a1     | /fe2e
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         bra     .+554           | /022a
|
|     otherwise
|         if dfstyle = floatdecimal then
         tstb   a3@
         bne     .+136           | /0088
|         begin
|             ds := copy(dsig, 1, df.digits);
         pea     a4@(4)           | /0004
         movw    #1,a7@-        | /0001
         movw    a3@(2),a7@-     | /0002
         pea     a6@(-488)        | /fe18
         bsr     _copy
         lea     a6@(-166),a0     | /ff5a
         lea     a6@(-488),a1     | /fe18
         moveq   #5,d0           | /05
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
|             i := d.exp + df.digits - 1;
         movw    a3@(2),d0        | /0002
         addw   a4@(2),d0        | /0002
         subqw  #1,d0           | /1
         movw    d0,d7
|             insert('.', ds, 2);
         pea pc@(524-2)           | /020c
         pea     a6@(-166)        | /ff5a
         movw    #2,a7@-        | /0002
         bsr     _ins
|             int2str(i, es);
         movw    d7,a7@-
         pea     a6@(-84)         | /ffac
         movl    a6,a7@-
         bsr     int2str
|             s := concat(s, ds, 'e', es)
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea     a6@(-166)        | /ff5a
         pea pc@(484-2)           | /01e4
         pea     a6@(-84)         | /ffac
         pea     a6@(-730)        | /fd26
         movw    #4,a7@-        | /0004
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-730),a1     | /fd26
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         bra     .+416           | /01a0
|         end
|         else
|         begin
|             {*
|             ** if it is a finite number, place a decimal
|             ** point after the lead digit and print the exponent field.
|             ** exponent must be adjusted to align point after lead digit.
|             ** three cases are handled, respectively:
|             **      xxxxxxxxx000..000.  (may be no zeros left of '.')
|             **      xxxxx.xxxxx
|             **      0.0000...000xxxxxxx (may be no zeros right of '.')
|             *}
|             if d.exp >= 0 then
         tstw   a4@(2)           | /0002
         blt     .+154           | /009a
|             begin
|                 s := concat(s, dsig);
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea     a4@(4)           | /0004
         pea     a6@(-832)        | /fcc0
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-832),a1     | /fcc0
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
|                 for i := 1 to d.exp do
         movw    a4@(2),a6@(-1368) | /0002, /faa8
         moveq   #1,d7           | /01
         bras   .+48            | /0030
|                     s := concat(s, '0');
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea pc@(366-2)           | /016e
         pea     a6@(-914)        | /fc6e
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-914),a1     | /fc6e
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         addqw  #1,d7           | /1
         bvss   .+8             | /0008
         cmpw   a6@(-1368),d7    | /faa8
         bles   .-50            | /0032
|                 s := concat(s, '.')
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea pc@(320-2)           | /0140
         pea     a6@(-996)        | /fc1c
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-996),a1     | /fc1c
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         bra     .+258           | /0102
|             end
|             else if (-d.exp) < length(dsig) then
         movw    a4@(2),d0        | /0002
         negw   d0
         movb    a4@(4),d1        | /0004
         andw  #255,d1         | /00ff
         cmpw   d0,d1
         bles   .+78            | /004e
|             begin
|                 insert('.', dsig, (length(d.sig) + d.exp + 1));
         pea pc@(266-2)           | /010a
         pea     a4@(4)           | /0004
         movb    a4@(4),d0        | /0004
         andw  #255,d0         | /00ff
         movw    a4@(2),d1        | /0002
         addw   d0,d1
         addqw  #1,d1           | /1
         movw    d1,a7@-
         bsr     _ins
|                 s := concat(s, dsig)
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea     a4@(4)           | /0004
         pea     a6@(-1098)       | /fbb6
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-1098),a1    | /fbb6
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         bra     .+164           | /00a4
|             end
|             else
|             begin
|                 s := concat(s, '0.');
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea pc@(176-2)           | /00b0
         pea     a6@(-1182)       | /fb62
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-1182),a1    | /fb62
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
|                 for i := 1 to -(length(dsig) + d.exp) do
         movb    a4@(4),d0        | /0004
         andw  #255,d0         | /00ff
         movw    a4@(2),d1        | /0002
         addw   d0,d1
         negw   d1
         movw    d1,a6@(-1370)    | /faa6
         moveq   #1,d7           | /01
         bras   .+48            | /0030
|                     s := concat(s, '0');
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea pc@(108-2)           | /006c
         pea     a6@(-1264)       | /fb10
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-1264),a1    | /fb10
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         addqw  #1,d7           | /1
         bvss   .+8             | /0008
         cmpw   a6@(-1370),d7    | /faa6
         bles   .-50            | /0032
|                 s := concat(s, dsig)
         movl    a6@(8),a0        | /0008
         movl    a0,a7@-
         movl    a6@(8),a7@-     | /0008
         pea     a4@(4)           | /0004
         pea     a6@(-1366)       | /faaa
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a7@+,a0
         lea     a6@(-1366),a1    | /faaa
         moveq   #20,d0          | /14
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
|             end
|         end
|     end
         moveml a7@+,#/1880
         unlk    a6
         movl    a7@+,a0
         addw  #12,a7          | /000c
         jmp     a0@
|
         .word   /c445,/4332,/5354,/5220  | ".ec2str "
         .word   /002c,/0130,/0230,/2e00  | ".,.0.0.."
         .word   /012e,/0145,/073f,/3f3f  | "...e.???"
         .word   /2e3f,/3f3f,/013a,/0127  | ".???.:.'"
         .word   /044e,/414e,/2700,/042b  | ".nan'..+"
         .word   /2b2b,/2b00,/042d,/2d2d  | "+++..---"
         .word   /2d00,/0330,/2e30        | "-..0.0"
| end;
|
|
|
|
| {*
| ** receive characters from "input stream" via character
| ** function nextc, which returns chr(0) = null when the
| ** string is exhausted.
| **
| ** the value the decimal structure represents is as
| ** follows (^ means exponentiation):
| **
| **      -1@^sgn * 10^exp * sig
| **
| ** the implied decimal point in the sig string is at the
| ** right-hand side.  infinities are read as strings of
| ** sign characters, + or -.  nans are strings of the form
| **      nan'xxxx:yyy...y'
| ** if there is a y-field, then the x-field is 0-padded on
| ** the left to width 4.  the colon is dropped from the string
| ** in dsig.
| *}
| procedure str2dec(s: decstr; var d: decimal);
| var
|     expadj, expmul, expmag, sigcnt, strindex, i : integer;
|     nc : char;          {input char buffer}
|     guard : boolean;    {for lost sig- and exp-digits}
|     ss : string[1];     {for character concatenations}
|
|
|     {*
|     ** get the next input character and increment string
|     ** index.  return null if string exhausted.
|     *}
|     function nextc: char;
|     begin
|
		.text
nextc:
         tstw   a7@(-4096)       | /f000
         link    a6,#0           | /0000
         movl    a4,a7@-
         movl    a6@(8),a4        | /0008
|         if strindex <= length(s) then
         movb    a4@(-82),d0      | /ffae
         andw  #255,d0         | /00ff
         cmpw   a4@(-86),d0      | /ffaa
         blts   .+28            | /001c
|         begin
|             nextc := s[strindex];
         movw    a4@(-86),d0      | /ffaa
         .word	/4e71,/4e71
         movb    a4@(-82,d0:w),d1 | /ae
         andw  #255,d1         | /00ff
         movw    d1,a6@(12)       | /000c
|             strindex := strindex + 1
         addqw  #1,a4@(-86)      | /1, /ffaa
         bras   .+12            | /000c
|         end
|         else
|             nextc := chr(0)
         moveq   #0,d0           | /00
         .word	/4e71,/4e71
         movw    d0,a6@(12)       | /000c
         movl    a7@+,a4
         unlk    a6
         movl    a7@+,a7@
         rts
|
         .word   /ce45,/5854,/4320,/2020  | ".extc   "
         .word   /0000                    | ".."
|     end;
|
|
| begin
|
		.text
		.globl	_str2dec
_str2dec:
|
|
         tstw   a7@(-4448)       | /eea0
         link    a6,#-352        | /fea0
         moveml #/f08,a7@-
         movl    a6@(12),a0       | /000c
         lea     a6@(-82),a1      | /ffae
         tstb   a0@
         moveq   #20,d0          | /14
         movl    a0@+,a1@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a0@,a1@
         movl    a6@(8),a4        | /0008
|     dsgn  := 0;        {assume positive}
         clrb   a4@
|     d.exp  := 0;        {default exponent}
         clrw   a4@(2)           | /0002
|     dsig  := '';       {default digit string is empty}
         clrb   a4@(4)           | /0004
|     guard  := false;    {nothing lost yet}
         clrb   d6
|
|     expadj := 0;        {loc of '.', relative to right digit}
         clrw   a6@(-94)         | /ffa2
|     expmul := 1;        {+1 or -1, depending on sign of exp}
         movw    #1,a6@(-92)      | /0001, /ffa4
|     expmag := 0;        {magnitude of exponent}
         clrw   a6@(-90)         | /ffa6
|
|     sigcnt := 0;        {number of digits seen so far}
         clrw   d4
|
|     ss     := '0';      {dummy 1-elt string}
         movw    #304,a6@(-100)   | /0130, /ff9c
|
|     strindex    := 1;        {beginning of string}
         movw    #1,a6@(-86)      | /0001, /ffaa
|
|     nc     := nextc;    {get first character}
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
|
|     {*
|     ** skip leading blanks and tabs (ascii 9).
|     *}
|     while (nc = ' ') or (nc = 9)@(chr) do
         cmpw  #32,d7          | /0020
         seq     d0
         moveq   #9,d1           | /09
         .word	/4e71,/4e71
         cmpw   d7,d1
         seq     d1
         orb    d1,d0
         beqs   .+14            | /000e
|         nc := nextc;
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
         bras   .-30            | /001e
|
|     {*
|     ** process sign, if any.
|     *}
|     if nc = '+' then
         cmpw  #43,d7          | /002b
         bnes   .+14            | /000e
|         nc := nextc
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
         bras   .+22            | /0016
|     else if nc = '-' then
         cmpw  #45,d7          | /002d
         bnes   .+16            | /0010
|     begin
|         dsgn := 1;  {mark negative}
         movb    #1,a4@         | /0001
|         nc := nextc
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
|     end;
|
|     {*
|     ** check for infinity, repeated sign.
|     *}
|     if ((nc = '+') and (dsgn = 0)) or ((nc = '-') and (d.sgn = 1)) then
         cmpw  #43,d7          | /002b
         seq     d0
         tstb   a4@
         seq     d1
         andb   d1,d0
         cmpw  #45,d7          | /002d
         seq     d1
         cmpb  #1,a4@         | /0001
         seq     d2
         andb   d2,d1
         orb    d1,d0
         beqs   .+12            | /000c
|         dsig := 'i'
         movw    #329,a4@(4)      | /0149, /0004
         bra     .+956           | /03bc
|
|     {*
|     ** check for 'nan''...''' or 'nan''...''', with a
|     ** hex significant digit field.
|     *}
|     else if (nc = 'n') or (nc = 'n') then
         cmpw  #78,d7          | /004e
         seq     d0
         cmpw  #110,d7         | /006e
         seq     d1
         orb    d1,d0
         beq     .+182           | /00b6
|     begin
|         { check whether more than following an'' }
|         if strindex > (length(s) - 4) then
         movb    a6@(-82),d0      | /ffae
         andw  #255,d0         | /00ff
         subqw  #4,d0           | /4
         cmpw   a6@(-86),d0      | /ffaa
         bges   .+18            | /0012
|             { if not, default to 0, to be replaced in dec2z }
|             dsig := 'n0000'
         movl    a4,a0
         addql  #4,a0           | /4
         lea pc@(942-2),a1        | /03ae
         movl    a1@+,a0@+
         movw    a1@,a0@
         bra     .+146           | /0092
|         else
|         begin
|             { copy up to 17 chars xxxx:yyyyyy... part to sig, after n }
|             i := length(s) - strindex - 3;
         movb    a6@(-82),d0      | /ffae
         andw  #255,d0         | /00ff
         subw   a6@(-86),d0      | /ffaa
         subqw  #3,d0           | /3
         movw    d0,d5
|             if i > 17 then
         cmpw  #17,d5          | /0011
         bles   .+4             | /0004
|                 i := 17;
         moveq   #17,d5          | /11
|             dsig := concat('n', copy(s, (strindex + 3), i));
         pea pc@(904-2)           | /0388
         pea     a6@(-82)         | /ffae
         movw    a6@(-86),d0      | /ffaa
         addqw  #3,d0           | /3
         movw    d0,a7@-
         movw    d5,a7@-
         pea     a6@(-182)        | /ff4a
         bsr     _copy
         pea     a6@(-182)        | /ff4a
         pea     a6@(-264)        | /fef8
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a4,a0
         addql  #4,a0           | /4
         lea     a6@(-264),a1     | /fef8
         moveq   #5,d0           | /05
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
|             { if there's a colon, force 4 lead digits }
|             i := pos(':', dsig);
         pea pc@(842-2)           | /034a
         pea     a4@(4)           | /0004
         bsr     _pos
         movw    a7@+,d5
|             if i > 0 then
         tstw   d5
         bles   .+42            | /002a
|             begin
|                 delete(dsig, i, 1);
         pea     a4@(4)           | /0004
         movw    d5,a7@-
         movw    #1,a7@-        | /0001
         bsr     _del
|                 while i < 6 do
         cmpw  #6,d5           | /0006
         bges   .+22            | /0016
|                 begin
|                     insert('0', dsig, 2);
         pea pc@(802-2)           | /0322
         pea     a4@(4)           | /0004
         movw    #2,a7@-        | /0002
         bsr     _ins
|                     i := i + 1
         addqw  #1,d5           | /1
         bras   .-24            | /0018
         bra     .+760           | /02f8
|                 end
|             end
|         end
|     end
|
|     {*
|     ** at last, it might be a number.  skip over leading zeros,
|     ** accept significant digits and discard extras with rounding
|     ** information saved in a system-dependent way.
|     ** mac: take sigdiglen-1 digits and append a '1' of nonzero
|     **          digits are lost.
|     ** iii: take sigdiglen digits and increment last (even if 9)
|     **          if nonzero digits are lost.
|     ** take care that discarded digits have appropriate effect on
|     ** exponent.
|     *}
|     else
|     begin
|
|         {*
|         ** if lead 0's are skipped, mark number tentatively as 0
|         *}
|         while nc = '0' do
         cmpw  #48,d7          | /0030
         bnes   .+20            | /0014
|         begin
|             nc := nextc;
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
|             dsig := '0'
         movw    #304,a4@(4)      | /0130, /0004
         bras   .-22            | /0016
|         end;
|
|         {*
|         ** beyond the leading 0's there may be: an honest
|         ** digit 1-9; a decimal point, possibly followed by
|         ** more 0's; or an exponent field, in which case the
|         ** result is zero.
|         *}
|         if (ord('1') <= nc@(ord)) and (nc@(ord) <= ord('9')) then
         cmpw  #49,d7          | /0031
         sge     d0
         cmpw  #57,d7          | /0039
         sle     d1
         andb   d1,d0
         beq     .+244           | /00f4
|         begin
|             sigcnt := 1;        {first sig digit}
         moveq   #1,d4           | /01
|             dsig := '0';       {make slot for nc}
         movw    #304,a4@(4)      | /0130, /0004
|             dsig[1] := nc;     {stuff nc}
         moveq   #1,d0           | /01
         movb    d7,a4@(4,d0:w)   | /04
|             nc := nextc;
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
|
|             while (ord('0') <= nc@(ord)) and (nc@(ord) <= ord('9')) do
         cmpw  #48,d7          | /0030
         sge     d0
         cmpw  #57,d7          | /0039
         sle     d1
         andb   d1,d0
         beqs   .+84            | /0054
|             begin
|                 {*
|                 ** mac vs iii difference in count:
|                 **      mac -- sigdiglen-1
|                 **      iii -- sigdiglen
|                 *}
|                 if (sigcnt < (sigdiglen-1)) then
         cmpw  #19,d4          | /0013
         bges   .+50            | /0032
|                 begin
|                     sigcnt := sigcnt + 1;
         addqw  #1,d4           | /1
|                     ss[1] := nc;
         moveq   #1,d0           | /01
         movb    d7,a6@(-100,d0:w) | /9c
|                     dsig := concat(d.sig, ss)
         pea     a4@(4)           | /0004
         pea     a6@(-100)        | /ff9c
         pea     a6@(-286)        | /fee2
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a4,a0
         addql  #4,a0           | /4
         lea     a6@(-286),a1     | /fee2
         moveq   #5,d0           | /05
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         bras   .+18            | /0012
|                 end
|                 else
|                 begin
|                     expadj := expadj + 1;
         addqw  #1,a6@(-94)      | /1, /ffa2
|                     guard := guard or (nc <> '0')
         cmpw  #48,d7          | /0030
         sne     d0
         orb    d0,d6
         andw  #1,d6           | /0001
|                 end;
|                 nc := nextc
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
         bras   .-96            | /0060
|             end;
|
|             {*
|             ** now the fraction part of the field may begin.
|             *}
|             if nc = '.' then
         cmpw  #46,d7          | /002e
         bnes   .+110           | /006e
|             begin
|                 nc := nextc;
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
|
|                 while (ord('0') <= nc@(ord)) and (nc@(ord) <= ord('9')) do
         cmpw  #48,d7          | /0030
         sge     d0
         cmpw  #57,d7          | /0039
         sle     d1
         andb   d1,d0
         beqs   .+84            | /0054
|                 begin
|                     if (sigcnt < (sigdiglen-1)) then
         cmpw  #19,d4          | /0013
         bges   .+54            | /0036
|                     begin
|                         sigcnt := sigcnt + 1;
         addqw  #1,d4           | /1
|                         expadj := expadj - 1;
         subqw  #1,a6@(-94)      | /1, /ffa2
|                         ss[1] := nc;
         moveq   #1,d0           | /01
         movb    d7,a6@(-100,d0:w) | /9c
|                         dsig := concat(d.sig, ss)
         pea     a4@(4)           | /0004
         pea     a6@(-100)        | /ff9c
         pea     a6@(-308)        | /fecc
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a4,a0
         addql  #4,a0           | /4
         lea     a6@(-308),a1     | /fecc
         moveq   #5,d0           | /05
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         bras   .+14            | /000e
|                     end
|                     else
|                         guard := guard or (nc <> '0');
         cmpw  #48,d7          | /0030
         sne     d0
         orb    d0,d6
         andw  #1,d6           | /0001
|                     nc := nextc
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
         bras   .-96            | /0060
         bra     .+168           | /00a8
|                 end
|             end
|         end
|
|         {*
|         ** in this case, we are still bypassing leading 0's
|         ** when the decimal point is found.
|         *}
|         else if nc = '.' then
         cmpw  #46,d7          | /002e
         bne     .+160           | /00a0
|         begin
|             nc := nextc;        {get following digit}
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
|
|             while nc = '0' do
         cmpw  #48,d7          | /0030
         bnes   .+24            | /0018
|             begin
|                 expadj := expadj - 1;
         subqw  #1,a6@(-94)      | /1, /ffa2
|                 dsig  := '0';
         movw    #304,a4@(4)      | /0130, /0004
|                 nc := nextc
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
         bras   .-26            | /001a
|             end;
|
|             if (ord('1') <= nc@(ord)) and (nc@(ord) <= ord('9')) then
         cmpw  #49,d7          | /0031
         sge     d0
         cmpw  #57,d7          | /0039
         sle     d1
         andb   d1,d0
         beqs   .+6             | /0006
|                 dsig := '';    {kill any zero string already found}
         clrb   a4@(4)           | /0004
|
|             while (ord('0') <= nc@(ord)) and (nc@(ord) <= ord('9')) do
         cmpw  #48,d7          | /0030
         sge     d0
         cmpw  #57,d7          | /0039
         sle     d1
         andb   d1,d0
         beqs   .+84            | /0054
|             begin
|                 if sigcnt < (sigdiglen-1) then
         cmpw  #19,d4          | /0013
         bges   .+54            | /0036
|                 begin
|                     expadj := expadj - 1;
         subqw  #1,a6@(-94)      | /1, /ffa2
|                     sigcnt := sigcnt + 1;
         addqw  #1,d4           | /1
|                     ss[1] := nc;
         moveq   #1,d0           | /01
         movb    d7,a6@(-100,d0:w) | /9c
|                     dsig := concat(d.sig, ss)
         pea     a4@(4)           | /0004
         pea     a6@(-100)        | /ff9c
         pea     a6@(-330)        | /feb6
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a4,a0
         addql  #4,a0           | /4
         lea     a6@(-330),a1     | /feb6
         moveq   #5,d0           | /05
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         bras   .+14            | /000e
|                 end
|                 else
|                     guard := guard or (nc <> '0');
         cmpw  #48,d7          | /0030
         sne     d0
         orb    d0,d6
         andw  #1,d6           | /0001
|                 nc := nextc
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
         bras   .-96            | /0060
|             end
|         end;
|
|         {*
|         ** if no legitimate digits, force nan and go through
|         ** the motions to get an exponent.  use #invd2b = 12
|         ** implicitly.
|         *}
|         if length(dsig) = 0 then
         movb    a4@(4),d0        | /0004
         andw  #255,d0         | /00ff
         tstw   d0
         bnes   .+10            | /000a
|             dsig := 'n12';
         movl    #55456050,a4@(4) | /034e3132, /0004
|
|         {*
|         ** strip trailing zeros, if no digits lost.
|         ** be careful not to delete all of zero string.
|         *}
|         if guard then
         movb    d6,d0
         beqs   .+50            | /0032
|         begin
|             {*
|             ** mac: append guard digit
|             ** iii: 'add' guard into last digit
|             *}
|             ss[1] := '1';
         moveq   #1,d0           | /01
         movb    #49,a6@(-100,d0:w) | /0031, /9c
|             dsig := concat(d.sig, ss)
         pea     a4@(4)           | /0004
         pea     a6@(-100)        | /ff9c
         pea     a6@(-352)        | /fea0
         movw    #2,a7@-        | /0002
         bsr     _cat
         movl    a4,a0
         addql  #4,a0           | /4
         lea     a6@(-352),a1     | /fea0
         moveq   #5,d0           | /05
         movl    a1@+,a0@+
         subqw  #1,d0           | /1
         bgts   .-4             | /0004
         movw    a1@,a0@
         bras   .+74            | /004a
|         end
|         else
|             while (length(dsig) > 1) and
         movb    a4@(4),d0        | /0004
         andw  #255,d0         | /00ff
         cmpw  #1,d0           | /0001
         sgt     d0
         movb    a4@(4),d1        | /0004
         andw  #255,d1         | /00ff
         .word	/4e71,/4e71
         movb    a4@(4,d1:w),d2   | /04
         andw  #255,d2         | /00ff
         cmpw  #48,d2          | /0030
         seq     d1
         andb   d1,d0
         beqs   .+30            | /001e
|                 (dsig[length(d.sig)] = '0') do
|             begin
|                 expadj := expadj + 1;
         addqw  #1,a6@(-94)      | /1, /ffa2
|                 delete(dsig, length(d.sig), 1)
|             end;
         pea     a4@(4)           | /0004
         movb    a4@(4),d0        | /0004
         andw  #255,d0         | /00ff
         movw    d0,a7@-
         movw    #1,a7@-        | /0001
         bsr     _del
         bras   .-70            | /0046
|
|
|         {*
|         ** now scan for an exponent field.
|         *}
|         if (nc = 'e') or (nc = 'e') then
         cmpw  #69,d7          | /0045
         seq     d0
         cmpw  #101,d7         | /0065
         seq     d1
         orb    d1,d0
         beq     .+134           | /0086
|         begin
|             expmul := 1;        {assume positive exp}
         movw    #1,a6@(-92)      | /0001, /ffa4
|             expmag := 0;
         clrw   a6@(-90)         | /ffa6
|
|             nc := nextc;
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
|
|             if (nc = '+') or (nc = '-') then
         cmpw  #43,d7          | /002b
         seq     d0
         cmpw  #45,d7          | /002d
         seq     d1
         orb    d1,d0
         beqs   .+24            | /0018
|             begin
|                 if nc = '-' then
         cmpw  #45,d7          | /002d
         bnes   .+8             | /0008
|                     expmul := -1;
         movw    #-1,a6@(-92)     | /ffff, /ffa4
|                 nc := nextc
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
|             end;
|
|             {*
|             ** use guard flag to catch outrageous exponents,
|             ** which are forced to 5000, a value guaranteed to
|             ** cause over/underflow, depending on expmul.
|             *}
|             guard := false;
         clrb   d6
|             while (ord('0') <= nc@(ord)) and (nc@(ord) <= ord('9')) do
         cmpw  #48,d7          | /0030
         sge     d0
         cmpw  #57,d7          | /0039
         sle     d1
         andb   d1,d0
         beqs   .+46            | /002e
|             begin
|                 guard := guard or (expmag >= 500);
         cmpw  #500,a6@(-90)    | /01f4, /ffa6
         sge     d0
         orb    d0,d6
         andw  #1,d6           | /0001
|                 expmag := (expmag * 10) + (nc@(ord) - ord('0'));
         moveq   #10,d0          | /0a
         muls    a6@(-90),d0      | /ffa6
         movl    d7,d1
         subw   #48,d1          | /0030
         addw   d0,d1
         movw    d1,a6@(-90)      | /ffa6
|                 nc := nextc
         clrw   a7@-
         movl    a6,a7@-
         bsr     nextc
         movw    a7@+,d7
         bras   .-58            | /003a
|             end;
|             if guard then
         movb    d6,d0
         beqs   .+8             | /0008
|                 expmag := 5000
         movw    #5000,a6@(-90)   | /1388, /ffa6
|         end;
|
|         {*
|         ** finish up number case by storing exponent, assuming
|         ** implicit decimal point to the right of the digit
|         ** string.
|         *}
|         d.exp := (expmul * expmag) + expadj;
         movw    a6@(-90),d0      | /ffa6
         muls    a6@(-92),d0      | /ffa4
         movw    a6@(-94),d1      | /ffa2
         addw   d0,d1
         movw    d1,a4@(2)        | /0002
|     end
         moveml a7@+,#/10f0
         unlk    a6
         movl    a7@+,a0
         addqw  #8,a7           | /8
         jmp     a0@
|
         .word   /d354,/5232,/4445,/4320  | ".tr2dec "
         .word   /000c,/0130,/013a,/014e  | "...0.:.n"
         .word   /054e,/3030,/3030        | ".n0000"
| end;
|

_srchk:  movl     a7@+,a7@
         rts      

_ins:    movl     a7@+,d1
         movw     a7@+,d0
         movl     a7@+,a1
         movl     a7@+,a0
         movl     d1,a7@-
         subqw   #/1,d0
         blts    .+/0030         | 0000003c
         clrw    d1
         movb     a0@+,d1
         clrw    d2
         movb     a1@,d2
         cmpw    d0,d2
         blts    .+/0024         | 0000003c
         movw     d1,d3
         addw    d2,d3
         movb     d3,a1@+
         movl     a1,a2
         addw   d3,a2
         movl     a1,a3
         addw   d2,a3
         subw    d0,d2
         bras    .+/0004         | 0000002e
         movb     a3@-,a2@-
         subqw   #/1,d2
         bpls    .-/0004         | 0000002c
         addw   d0,a1
         bras    .+/0004         | 00000038
         movb     a0@+,a1@+
         subqw   #/1,d1
         bpls    .-/0004         | 00000036
         rts      
  
_del:    movl     a7@+,a1
         movw     a7@+,d0
         movw     a7@+,d1
         movl     a7@+,a0
         movl     a1,a7@-
         cmpw   #/0000,d0
         bles    .+/0032         | 00000040
         cmpw   #/0000,d1
         bles    .+/002c         | 00000040
         clrw    d2
         movb     a0@,d2
         cmpw    d1,d2
         blts    .+/0024         | 00000040
         movw     d1,d3
         addw    d0,d3
         subqw   #/1,d3
         subw    d2,d3
         bles    .+/0008         | 0000002e
         subqw   #/1,d1
         movb     d1,a0@
         bras    .+/0014         | 00000040
         subw    d0,d2
         movb     d2,a0@
         addw   d1,a0
         movl     a0,a1
         addw   d0,a1
         bras    .+/0004         | 0000003c
         movb     a1@+,a0@+
         addqw   #/1,d3
         bles    .-/0004         | 0000003a
         rts      

_copy:   moveml  #/e0c0,a7@-
         movl     a7@(/0018),a0
         movw     a7@(/001c),d0
         movw     a7@(/001e),d1
         subqw   #/1,d1
         movl     a7@(/0020),a1
         clrw    d2
         movb     a1@+,d2
         subw    d1,d2
         subw    d0,d2
         blts    .+/0020         | 0000003e
         addw   d1,a1
         movb     d0,a0@+
         bras    .+/0004         | 00000028
         movb     a1@+,a0@+
         subqw   #/1,d0
         bpls    .-/0004         | 00000026
         movl     a7@(/0014),a7@(/020)
         moveml  a7@+,#D0+D1+D2+A0+A1
         addl   #/0000000c,a7
         rts      
         clrb    a0@
         bras    .-/0014         | 0000002c

_cat:   moveml  #/e0e0,a7@-
         movw     a7@(/001c),d0
         movl     a7@(/001e),a0
         lea      a7@(/0022),a1
         movw     d0,d2
         lslw    #/2,d2
         addw   d2,a1
         clrw    d1
         addql   #/1,a0
         bras    .+/0012         | 0000002c
         movl     a1@-,a2
         clrw    d2
         movb     a2@+,d2
         addw    d2,d1
         bras    .+/0004         | 00000028
         movb     a2@+,a0@+
         subqw   #/1,d2
         bpls    .-/0004         | 00000026
         subqw   #/1,d0
         bpls    .-/0012         | 0000001c
         movl     a7@(/001e),a0
         movb     d1,a0@
         lea      a7@(/0022),a1
         movw     a7@(/001c),d0
         lslw    #/2,d0
         addw   d0,a1
         movl     a7@(/0018),a1@-
         movl     a1,a7@(/0018)
         moveml  a7@+,#D0+D1+D2+A0+A1+A2
         movl     a7@+,a7
         rts      

_pos:   moveml  #/e0e0,a7@-
         movl     a7@(/001c),a0
         clrw    d0
         movb     a0@+,d0
         clrw    d1
         movl     a7@(/0020),a1
         clrw    d2
         movb     a1@+,d2
         cmpw    d2,d0
         bges    .+/0006         | 0000001e
         clrw    d1
         bras    .+/0018         | 00000034
         movl     a0,a2
         addql   #/1,d1
         bras    .+/0006         | 00000028
         cmpmb   a2@+,a1@+
         bnes    .+/0008         | 0000002e
         subqw   #/1,d2
         bpls    .-/0006         | 00000024
         bras    .+/0008         | 00000034
         addql   #/1,a0
         subqw   #/1,d0
         bras    .-/0024         | 0000000e
         movw     d1,a7@(/0022)
         movl     a7@(/0018),a7@(/001e)
         moveml  a7@+,#D0+D1+D2+A0+A1+A2
         addql   #/6,a7
         rts      
