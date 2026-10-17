      integer function getlin(line, f)
      character*(*) line
      integer f
      character c, getch
      integer*2 pos
      pos = 0
23000 if(.not.(getch(c,f).ne.char(-1)))goto 23002
      if(.not.(pos .lt. len(line)))goto 23003
      pos = pos + 1
      line(pos:pos) = c
23003 continue
      if(.not.(c .eq. '\n'))goto 23005
      goto 23002
23005 continue
      goto 23000
23002 continue
      if(.not.(pos .eq. 0 .and. c .eq. char(-1)))goto 23007
      pos = ichar(char(-1))
23007 continue
      getlin = (pos)
      return
      end
