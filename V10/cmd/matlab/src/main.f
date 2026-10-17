C     PROGRAM MAIN
      integer kfault
      integer jmpbuf(10),rv
      common /sjmp/ jmpbuf,rv
      external onbrk
C
C** WARNING***
C** ofault only works for floating point overflow faults, which occur on
C** all VAXes, EXCEPT 11/780s before Engineering Change Order 7.
C** If you have an old 11/780 which has not yet been "fixed" by DEC,
C** either ask them to fix it, or get our old handler for traps.
C**     --Moler, 7/13/83
C
      call ofault(50)
      rv = 0
      call setjmp(jmpbuf)
      call sgset(2,onbrk)
      call matlab(rv)
      k = kfault(0)
      if (k .gt. 0) write(6,13) k
   13 format('total overflows ',i9/)
      stop
      end
