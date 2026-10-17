PROGRAM Fromhex;

{$U-}
{$X-}
{$D+}
{$R-}
{$L-}

USES {$U Obj/Quickdraw } Quickdraw,
     {$U Obj/OSIntf    } OSIntf,
     {$U Obj/ToolIntf  } ToolIntf;

{$L+}
{$ASM+}

CONST bufsize = 512;
      mask = 127;

VAR rout, rin, io, i: INTEGER;
    szin, l, val, bytes, count, sum, getsum: LongInt;
    nout, nin: OsStr255;
    bin, bout: ARRAY [1..bufsize] OF -128..127;
    bini, bouti: INTEGER;
    {debugger: text;}

PROCEDURE putflush;
BEGIN
  if bouti <> 1 then
  BEGIN
    count := bouti - 1;
    io := FSWrite(rout, count, @bout);
    bouti := 1;
  END;
END;

PROCEDURE putchar (ch:LongInt);
BEGIN
  bout[bouti] := ch;
  bouti := bouti + 1;
  if bouti > bufsize then putflush;
END;

FUNCTION getchar : LongInt;
VAR t: LongInt;
BEGIN
  if bini > bufsize then
  BEGIN
    bini := 1;
    count := bufsize;
    io := FSRead(rin, count, @bin);
  END;
  t := ord4(bin[bini]);
  bini := bini + 1;
  {write(debugger, 'getc ', t:4, '  ');}
  getchar := t;
END;

PROCEDURE badnews;              {if I was smart, I'd use a dialog box...}
VAR j: INTEGER;
BEGIN
  io := FSClose(rin);
  io := FSClose(rout);
  sysbeep(1);
  for j := 1 to 20000 do l:=0;
  sysbeep(1);
  exittoshell;
END;

BEGIN                   {main program}
  nout := 'C prog';
  nin := 'C';
  bouti := 1;
  bini := bufsize + 1;
  {Rewrite (debugger, '.BOUT'); the serial port not used for downloading from Lisa}

  io := FSDelete(nout, 0);
  io := Create(nout, 0, 'CCOM', 'APPL');
  io := OpenRF(nout, 0, rout);
  if io <> 0 then badnews;
  io := FSOpen(nin, 0, rin);
  if io <> 0 then badnews;
  io := GetEOF(rin, szin);
  val := 0;
  bytes := 0;
  sum := 0;
  getsum := 0;

  WHILE ((szin > 0) AND (getsum = 0)) DO
  BEGIN
    l := getchar;
    szin := szin - 1;
    l := BitAnd(l, ord4(mask));
    if (l >= 64) and (l < 80) then
    BEGIN
      bytes := bytes + 1;
      l := l - 64;
      val := BitShift(val, 4);
      val := BitOr(val, l);
      {write(debugger, 'val ', val:4, '  ');}
      if BitAnd(bytes, ord4(1)) = 0 then
      BEGIN
        putchar(val);
        sum := sum + val;
        val := 0;
      END;
    END;
    if (l = 124) then getsum := 1;  { we found "|" }
  END;
  putflush;
  io := FSClose(rout);
  {writeln(debugger, 'bytes', bytes, 'sum', sum);}
  if getsum = 0 then badnews;
  sum := sum + BitShift(bytes, -1);
  val := 0;
  for i := 1 to 8 do
  BEGIN
    l := getchar;
    l := BitAnd(l, ord4(15));
    val := BitShift(val, 4);
    val := BitOr(val, l);
  END;
  io := FSClose(rin);
  if val <> sum then badnews;
  exittoshell;          {it worked!}
END.
