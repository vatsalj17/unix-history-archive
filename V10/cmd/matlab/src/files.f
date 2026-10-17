      SUBROUTINE FILES(LUNIT,NAME,IOSTAT)
      INTEGER LUNIT,NAME(64),IOSTAT
C
C     SYSTEM DEPENDENT ROUTINE TO ALLOCATE FILES
C     LUNIT = LOGICAL UNIT NUMBER
C           = 1,  SAVE
C           = 2,  LOAD
C           = 7,  PRINT
C           = 8,  DIARY
C           > 10, EXEC
C           < 0,  CLOSE -LUNIT
C           = -5, SPECIAL CASE, END OF FILE DETECTED ON TERMINAL
C     NAME = FILE NAME, 1 CHARACTER PER WORD
C     NONZERO IOSTAT RETURNED FOR ERROR CONDITION
C
C     (UNLESS CHANGED IN SUBROUTINE MATLAB, UNITS 5, 6 AND 9 ARE
C      USED FOR TERMINAL INPUT, TERMINAL OUTPUT AND THE HELP FILE.
C      THE HELP FILE IS OPENED BY SUBROUTINE HELPER.)
C
      character*64 nam
c
      if (lunit .lt. 0) go to 30
c
c     Fortran 77 internal file conversion from 64a1 to character*64
c
      write(nam,'(64a1)') name
c
c     unformatted i/o for save and load
c     formatted i/o for exec, diary and print
c
      iostat = 0
      if (lunit .eq. 1)  open(unit=lunit,file=nam,form='unformatted',
     >                   status='new',err=20,iostat=iostat)
      if (lunit .eq. 2)  open(unit=lunit,file=nam,form='unformatted',
     >                   status='old',err=20,iostat=iostat)
      if (lunit .eq. 7)  open(unit=lunit,file=nam,
     >                   status='new',err=20,iostat=iostat)
      if (lunit .eq. 8)  open(unit=lunit,file=nam,
     >                   status='new',err=20,iostat=iostat)
      if (lunit .gt. 10) open(unit=lunit,file=nam,
     >                   status='old',err=20,iostat=iostat)
      if (iostat .ne. 0) go to 20
c
c     rewind all except diary
c
      if (lunit .ne. 8) rewind lunit
      return
c
c     error on open
c
   20 if (iostat .eq. 0) iostat = -1
      return
c
c     close files
c
   30 if (lunit .eq. -5) go to 50
      close(unit=-lunit)
      return
c
c     UNIX special case -- EOF from terminal implies stop
c
   50 write(6,51)
   51 format(/'EOF on terminal input')
      stop
      end
