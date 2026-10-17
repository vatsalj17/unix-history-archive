      DOUBLE PRECISION FUNCTION WNRM2(N,XR,XI,INCX)
      DOUBLE PRECISION XR(1),XI(1),PYTHAG,S
C     NORM2(X)
      S = 0.0D0
      IF (N .LE. 0) GO TO 20
      IX = 1
      DO 10 I = 1, N
         S = PYTHAG(S,XR(IX))
         S = PYTHAG(S,XI(IX))
         IX = IX + INCX
   10 CONTINUE
   20 WNRM2 = S
      RETURN
      END
