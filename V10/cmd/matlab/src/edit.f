      SUBROUTINE EDIT(BUF,N)
      INTEGER BUF(N)
      character*256 s
      open(7,file='/tmp/matlab.vi')
      write(7,10) (buf(i),i=1,n)
   10 format(256a1)
      close(7)
      call system('vi /tmp/matlab.vi')
      open(7,file='/tmp/matlab.vi')
      rewind 7
      read(7,'(a)') s
      close(7)
      call system('rm /tmp/matlab.vi')
      n = 0
      do 20 i = 1, 256
         if (s(i:i) .ne. ' ') n = i
   20 continue
      call prompt(0)
      write(6,'(a)') s(1:n)
      read(s,10) (buf(i),i=1,n)
      return
      end
