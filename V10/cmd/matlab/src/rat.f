      SUBROUTINE RAT(X,LEN,MAXD,A,B,D)
      INTEGER LEN,MAXD
      DOUBLE PRECISION X,A,B,D(LEN)
C
C     A/B = CONTINUED FRACTION APPROXIMATION TO X
C           USING  LEN  TERMS EACH LESS THAN MAXD
C 
      INTEGER DDT,ERR,FMT,LCT(4),LIN(1024),LPT(6),RIO,WIO,RTE,WTE,HIO
      COMMON /IOP/ DDT,ERR,FMT,LCT,LIN,LPT,RIO,WIO,RTE,WTE,HIO
      DOUBLE PRECISION S,T,Z,ROUND 
      Z = X
      S = MAXD
      DO 10 I = 1, LEN
         K = I
         D(K) = ROUND(Z)  
         Z = Z - D(K)
         IF (S*DABS(Z) .LE. 1.0D0) GO TO 20
         Z = 1.0D0/Z
   10 CONTINUE
   20 T = D(K)
      S = 1.0D0
      IF (K .LT. 2) GO TO 40
      DO 30 IB = 2, K
         I = K+1-IB
         Z = T
         T = D(I)*T + S
         S = Z
   30 CONTINUE
   40 IF (S .LT. 0.0D0) T = -T
      IF (S .LT. 0.0D0) S = -S
      IF (DDT .EQ. 27) WRITE(WTE,50) X,T,S,(D(I),I=1,K)
   50 FORMAT(/0X,1PD23.15,0PF8.0,' /',F8.0,4X,6F5.0/(0X,45X,6F5.0))
      A = T
      B = S
      RETURN
      END
