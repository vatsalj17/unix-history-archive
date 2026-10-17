      program hlpset
c
c     Two direct access files are required.  Their locations are:
c
      character*(*) idfile,dafile
      parameter(idfile='/usr/local/lib/mathelp.idx')
      parameter(dafile='/usr/local/lib/mathelp.dac')
c
      character*66 line
      open(unit=9,file='help',access='SEQUENTIAL',status='OLD')
      open(unit=1,file=dafile,access='DIRECT',recl=66,status='NEW')
      open(unit=2,file=idfile,access='DIRECT',recl=8,status='NEW')
      rewind 9
      n = 0
      k = 0
   10 read(9,12,end=20) line
   12 format(a66)
      n = n + 1
      write(1,rec=n) line
      if (line(1:4) .ne. '    ') then
         k = k + 1
         write(2,rec=k) line(1:4),n
      endif
      if (line(1:4) .ne. 'EOF ') go to 10
   20 close(1)
      close(2)
      end
