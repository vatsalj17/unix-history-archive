      subroutine helper(h)
      character*4 h(4)
c
c     Handle HELP file
c     h = key word, format 4a1
c
c     Two direct access files are required.  Their locations are:
c
      character*(*) idfile,dafile
      parameter(idfile='/v/lib/mathelp.idx')
      parameter(dafile='/v/lib/mathelp.dac')
c
      parameter(lrecl=66)
      character*66 line
      character*4 keys(128),key
      integer locs(128)
      INTEGER DDT,ERR,FMT,LCT(4),LIN(1024),LPT(6),RIO,WIO,RTE,WTE,HIO
      COMMON /IOP/ DDT,ERR,FMT,LCT,LIN,LPT,RIO,WIO,RTE,WTE,HIO
c
c     Initialization indicated by h(1) = 0
c
      if (h(1)(1:1) .eq. char(0)) then
         open(unit=HIO,file=idfile,access='DIRECT',recl=8)
         n = 0
   10    n = n + 1
         read(HIO,rec=n,err=15) keys(n),locs(n)
         if (keys(n) .ne. 'EOF ') go to 10
   15    close(unit=HIO)
         nkeys = n-1
         key = 'HELP'
         open(unit=HIO,file=dafile,access='DIRECT',recl=lrecl)
      endif
c
c     Convert h to conventional string
c
      if (h(1)(1:1) .ne. char(0)) then
         do 20 i = 1, 4
            key(i:i) = h(i)
   20    continue
      endif
c
c     Special case, HELP followed by blank.  Give general help.
c
      if (key .eq. '    ') then
         write(WTE,30)
         if (WIO .ne. 0) write(WIO,30)
   30    format(0X,'Type HELP followed by'
     >      /0X,'INTRO   (To get started)'
     >      /0X,'NEWS    (recent revisions)')
         write(WTE,31) (keys(n), n = 19, nkeys)
         if (WIO .ne. 0) write(WIO,31) (keys(n), n = 19, nkeys)
   31    format(0X,a4,10a6)
         write(WTE,32) (keys(n), n = 4, 18)
         if (WIO .ne. 0) write(WIO,32) (keys(n), n = 4, 18)
   32    format(0X,15(a1,1x))
         return
      endif
c
c     Find key word in keys table
c
      do 40 k = 1, nkeys
         if (key .eq. keys(k)) go to 50
   40 continue
      write(WTE,45) key
      if (WIO .ne. 0) write(WIO,45) key
   45 format(0X,'SORRY, NO HELP ON ',a4)
      return
c
c     Read and echo lines from direct access help file
c
   50 write(WTE,75)
      if (WIO .ne. 0) write(WIO,75)
      k0 = locs(k)
      k1 = locs(k+1) - 2
      do 80 k = k0, k1
   60    read(HIO,rec=k) line
         do 65 j = lrecl, 1, -1
            if (line(j:j) .ne. ' ') go to 70
   65    continue
   70    write(WTE,75) line(1:j)
         if (WIO .ne. 0) write(WIO,75) line(1:j)
   75    format(0X,a)
   80 continue
      return
      end
