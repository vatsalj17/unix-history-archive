      SUBROUTINE XCHAR(BUF,K)
      INTEGER BUF(1),K
C
C     SYSTEM DEPENDENT ROUTINE TO HANDLE SPECIAL CHARACTERS
C
      INTEGER DDT,ERR,FMT,LCT(4),LIN(1024),LPT(6),RIO,WIO,RTE,WTE,HIO
      COMMON /IOP/ DDT,ERR,FMT,LCT,LIN,LPT,RIO,WIO,RTE,WTE,HIO
      character*80 s
      character*1 bang,tab,blank
      integer blankk
      data bang/'!'/,tab /'	'/,blank/' '/,blankk/37/
c
c     Exclamation mark, pass rest of line on to UNIX
c
      if (char(buf(1)) .eq. bang) then
         do 10 i = 1, 79
            s(i:i) = char(buf(i+1))
   10    continue
         s(80:80) = blank
         call system(s)
         k = 99
c
c     Replace tab by blank
c
      else if (char(buf(1)) .eq. tab) then
         k = blankk
c
c     Anything else produces error message
c
      else
         write(WTE,20) buf(1)
   20    format(A1,' is not a MATLAB character.')
c
      endif
      RETURN
      END
