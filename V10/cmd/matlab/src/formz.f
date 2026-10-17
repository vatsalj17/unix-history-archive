      SUBROUTINE FORMZ(LUNIT,X,Y)
      DOUBLE PRECISION X,Y
C
C     SYSTEM DEPENDENT ROUTINE TO PRINT WITH Z FORMAT
C
C     IF (Y .NE. 0.0D0) WRITE(LUNIT,10) X,Y
C     IF (Y .EQ. 0.0D0) WRITE(LUNIT,10) X
C  10 FORMAT(2Z18)
C
      double precision xx
      integer ix(2),nohi,sig,p(16)
      character*1 d(16),s(16)
      equivalence (xx,ix)
      data d/'0','1','2','3','4','5','6','7',
     >       '8','9','A','B','C','D','E','F'/
      data p/4,3,2,1,8,7,6,5,12,11,10,9,16,15,14,13/
      data nohi/z'7fffffff'/
      xx = x
      k = 1
      do 20 i = 1, 2
         m = ix(i)
         if (m .lt. 0) then
            sig = 1
            m = and(m,nohi)
         else
            sig = 0
         endif
         do 20 j = 1, 8
            l = mod(m,16)+1
            m = m/16
            if (j.eq.8 .and. sig.eq.1) l = l+8
            s(p(k)) = d(l)
            k = k+1
   20 continue
      write(lunit,'(6x,16a1)') s
      RETURN
      END
