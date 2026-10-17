      SUBROUTINE MATLAB(INIT)
C     INIT = 0 FOR ORDINARY FIRST ENTRY 
C          = POSITIVE FOR SUBSEQUENT ENTRIES
C          = NEGATIVE FOR SILENT INITIALIZATION (SEE MATZ)
C
      DOUBLE PRECISION STKR(50005),STKI(50005)
      INTEGER IDSTK(4,48),LSTK(48),MSTK(48),NSTK(48),VSIZE,LSIZE,BOT,TOP
      INTEGER ALFA(52),ALFB(52),ALFL,CASE
      INTEGER IDS(4,32),PSTK(32),RSTK(32),PSIZE,PT,PTZ
      INTEGER DDT,ERR,FMT,LCT(4),LIN(1024),LPT(6),RIO,WIO,RTE,WTE,HIO
      INTEGER SYM,SYN(4),BUF(256),CHAR,FLP(2),FIN,FUN,LHS,RHS,RAN(2)
      COMMON /VSTK/ STKR,STKI,IDSTK,LSTK,MSTK,NSTK,VSIZE,LSIZE,BOT,TOP
      COMMON /ALFS/ ALFA,ALFB,ALFL,CASE
      COMMON /RECU/ IDS,PSTK,RSTK,PSIZE,PT,PTZ
      COMMON /IOP/ DDT,ERR,FMT,LCT,LIN,LPT,RIO,WIO,RTE,WTE,HIO
      COMMON /COM/ SYM,SYN,BUF,CHAR,FLP,FIN,FUN,LHS,RHS,RAN
C
      DOUBLE PRECISION S,T
      INTEGER EPS(4),FLOPS(4),EYE(4),RAND(4)
C
C     CHARACTER SET
C            0       10       20       30       40       50
C
C     0      0        A        K        U   COLON  :  LESS   <
C     1      1        B        L        V   PLUS   +  GREAT  >
C     2      2        C        M        W   MINUS  -
C     3      3        D        N        X   STAR   *
C     4      4        E        O        Y   SLASH  /
C     5      5        F        P        Z   BSLASH \\
C     6      6        G        Q  BLANK     EQUAL  =
C     7      7        H        R  LPAREN (  DOT    .
C     8      8        I        S  RPAREN )  COMMA  ,
C     9      9        J        T  SEMI   ;  QUOTE  '
C
      INTEGER ALPHA(52),ALPHB(52)
      DATA ALPHA /1H0,1H1,1H2,1H3,1H4,1H5,1H6,1H7,1H8,1H9,
     $    1HA,1HB,1HC,1HD,1HE,1HF,1HG,1HH,1HI,1HJ,
     $    1HK,1HL,1HM,1HN,1HO,1HP,1HQ,1HR,1HS,1HT,
     $    1HU,1HV,1HW,1HX,1HY,1HZ,1H ,1H(,1H),1H;,
     $    1H:,1H+,1H-,1H*,1H/,1H\\,1H=,1H.,1H,,1H',
     $    1H<,1H>/
C
C     ALTERNATE CHARACTER SET
C
      DATA ALPHB /1H0,1H1,1H2,1H3,1H4,1H5,1H6,1H7,1H8,1H9,
     $    1Ha,1Hb,1Hc,1Hd,1He,1Hf,1Hg,1Hh,1Hi,1Hj,
     $    1Hk,1Hl,1Hm,1Hn,1Ho,1Hp,1Hq,1Hr,1Hs,1Ht,
     $    1Hu,1Hv,1Hw,1Hx,1Hy,1Hz,1H ,1H(,1H),1H;,
     $    1H|,1H+,1H-,1H*,1H/,1H$,1H=,1H.,1H,,1H",
     $    1H[,1H]/
C
      DATA EPS/14,25,28,36/,FLOPS/15,21,24,25/
      DATA EYE/14,34,14,36/,RAND/27,10,23,13/
C
      IF (INIT .GT. 0) GO TO 90
C
C     RTE = UNIT NUMBER FOR TERMINAL INPUT
C     WTE = UNIT NUMBER FOR TERMINAL OUTPUT
C     HIO = UNIT NUMBER FOR HELP FILE
C
      RTE = 5
      WTE = 6
      HIO = 9
      RIO = RTE
      WIO = 0
C
      IF (INIT .GE. 0) WRITE(WTE,100)
  100 FORMAT(//0X,'     < M A T L A B >'
     $  /0X,'   Version of 11/01/83')
C
C     ASK HELPER TO OPEN HELP FILE
      BUF(1) = 0
      CALL HELPER(BUF)
C
C     RANDOM NUMBER SEED
      RAN(1) = 0
C
C     INITIAL LINE LIMIT
      LCT(2) = 25000
C
      ALFL = 52
      CASE = 1
C     CASE = 1 for file names in lower case
      DO 20 I = 1, ALFL
         ALFA(I) = ALPHA(I)
         ALFB(I) = ALPHB(I)
   20 CONTINUE
C
      VSIZE = 50005
      LSIZE = 48
      PSIZE = 32
      BOT = LSIZE-3
      CALL WSET(5,0.0D0,0.0D0,STKR(VSIZE-4),STKI(VSIZE-4),1)
      CALL PUTID(IDSTK(1,LSIZE-3),EPS)
      LSTK(LSIZE-3) = VSIZE-4
      MSTK(LSIZE-3) = 1
      NSTK(LSIZE-3) = 1
      S = 1.0D0
   30 S = S/2.0D0
      T = 1.0D0 + S
      IF (T .GT. 1.0D0) GO TO 30
      STKR(VSIZE-4) = 2.0D0*S
      CALL PUTID(IDSTK(1,LSIZE-2),FLOPS)
      LSTK(LSIZE-2) = VSIZE-3
      MSTK(LSIZE-2) = 1
      NSTK(LSIZE-2) = 2
      CALL PUTID(IDSTK(1,LSIZE-1), EYE)
      LSTK(LSIZE-1) = VSIZE-1
      MSTK(LSIZE-1) = -1
      NSTK(LSIZE-1) = -1
      STKR(VSIZE-1) = 1.0D0
      CALL PUTID(IDSTK(1,LSIZE), RAND)
      LSTK(LSIZE) = VSIZE
      MSTK(LSIZE) = 1
      NSTK(LSIZE) = 1
      FMT = 1
      FLP(1) = 0
      FLP(2) = 0
      DDT = 0
      RAN(2) = 0
      PTZ = 0
      PT = PTZ
      ERR = 0
      IF (INIT .LT. 0) RETURN
C
   90 CALL PARSE
      IF (FUN .EQ. 1) CALL MATFN1
      IF (FUN .EQ. 2) CALL MATFN2
      IF (FUN .EQ. 3) CALL MATFN3
      IF (FUN .EQ. 4) CALL MATFN4
      IF (FUN .EQ. 5) CALL MATFN5
      IF (FUN .EQ. 6) CALL MATFN6
      IF (FUN .EQ. 21) CALL MATFN1
      IF (FUN .NE. 99) GO TO 90
      RETURN
      END
