      DOUBLE PRECISION FUNCTION FLOP(X)
      DOUBLE PRECISION X
C     SYSTEM DEPENDENT FUNCTION
C     COUNT AND POSSIBLY CHOP EACH FLOATING POINT OPERATION
C     FLP(1) IS FLOP COUNTER
C     FLP(2) IS NUMBER OF PLACES TO BE CHOPPED
C
      INTEGER SYM,SYN(4),BUF(256),CHAR,FLP(2),FIN,FUN,LHS,RHS,RAN(2)
      COMMON /COM/ SYM,SYN,BUF,CHAR,FLP,FIN,FUN,LHS,RHS,RAN
C
      DOUBLE PRECISION MASK(14),XX,MM
      real mas(2,14)
      integer xexp,ibmask(3)
      integer lx(2),lm(2)
      equivalence (lx(1),xx),(lm(1),mm)
      equivalence (mask(1),mas(1))
      data mas/
     $ z'ffffffff',z'fff0ffff',
     $ z'ffffffff',z'ff00ffff',
     $ z'ffffffff',z'f000ffff',
     $ z'ffffffff',z'0000ffff',
     $ z'ffffffff',z'0000fff0',
     $ z'ffffffff',z'0000ff00',
     $ z'ffffffff',z'0000f000',
     $ z'ffffffff',z'00000000',
     $ z'fff0ffff',z'00000000',
     $ z'ff00ffff',z'00000000',
     $ z'f000ffff',z'00000000',
     $ z'0000ffff',z'00000000',
     $ z'0000fff0',z'00000000',
     $ z'0000ff80',z'00000000'/
      data xexp/z'00000180'/
      data ibmask/z'fff8ffff',z'fffcffff',z'fffeffff'/
C
      FLP(1) = FLP(1) + 1
      K = FLP(2)
      FLOP = X
      IF (K .LE. 0) RETURN
      if (k .eq. 360) go to 360
      if (k .eq. 541) go to 541
      FLOP = 0.0D0
      IF (K .GE. 15) RETURN
      XX = X
      MM = MASK(K)
      LX(1) = and(LX(1),LM(1))
      LX(2) = and(LX(2),LM(2))
      FLOP = XX
      RETURN
c
c     simulate ibm/360 arithmetic
c     look at last two bits of exponent and
c     chop 0, 1, 2 or 3 bits off the fraction
c
  360 xx = x
      k = and(lx(1),xexp)/128
      if (k .gt. 0) lx(2) = and(lx(2),ibmask(k))
      flop = xx
      return
c
c     the lobo 541, 2 digit decimal fraction
c
  541 xx = dabs(x)
      if (xx .gt. 9.9d0) xx = 9.9d0
      if (xx .lt. 0.01d0) xx = 0.0d0
      if (xx .ge. 1.0d0) xx = round(10.d0*xx)/10.d0
      if (xx.ge.0.1d0 .and. xx.lt.1.0d0) xx = round(100.d0*xx)/100.d0
      if (xx .lt. 0.1d0) xx = round(1000.d0*xx)/1000.d0
      if (x .lt. 0.0d0) xx = -xx
      flop = xx
      return
      END
