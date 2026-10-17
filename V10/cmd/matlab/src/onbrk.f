      function onbrk
      common /sjmp/jmpbuf(10),rv
      rv = 1
      write(6,*) "Interrupt"
      call longjp(jmpbuf)
      end

