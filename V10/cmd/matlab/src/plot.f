      SUBROUTINE PLOT(LUNIT,X,Y,N,P,K,BUF)
      DOUBLE PRECISION X(N),Y(N),P(1)
      INTEGER BUF(79)
C
C     PLOT X VS. Y ON LUNIT
C     IF K IS NONZERO, THEN P(1),...,P(K) ARE EXTRA PARAMETERS
C     BUF IS WORK SPACE
C
      DOUBLE PRECISION XMIN,YMIN,XMAX,YMAX,DY,DX,Y1,Y0
      INTEGER AST,BLANK,H,W
      DATA AST/1H*/,BLANK/1H /,H/20/,W/79/
C
C     H = HEIGHT, W = WIDTH
C
      XMIN = X(1)
      XMAX = X(1)
      YMIN = Y(1)
      YMAX = Y(1)
      DO 10 I = 1, N
         XMIN = DMIN1(XMIN,X(I))
         XMAX = DMAX1(XMAX,X(I))
         YMIN = DMIN1(YMIN,Y(I))
         YMAX = DMAX1(YMAX,Y(I))
   10 CONTINUE
      DX = XMAX - XMIN
      IF (DX .EQ. 0.0D0) DX = 1.0D0
      DY = YMAX - YMIN
      WRITE(LUNIT,35)
      DO 40 L = 1, H
         DO 20 J = 1, W
            BUF(J) = BLANK
   20    CONTINUE
         Y1 = YMIN + (H-L+1)*DY/H
         Y0 = YMIN + (H-L)*DY/H
         JMAX = 1
         DO 30 I = 1, N
            IF (Y(I) .GT. Y1) GO TO 30
            IF (L.NE.H .AND. Y(I).LE.Y0) GO TO 30
            J = 1 + (W-1)*(X(I) - XMIN)/DX
            BUF(J) = AST
            JMAX = MAX0(JMAX,J)
   30    CONTINUE
         WRITE(LUNIT,35) (BUF(J),J=1,JMAX)
   35    FORMAT(79A1)
   40 CONTINUE
      RETURN
      END
