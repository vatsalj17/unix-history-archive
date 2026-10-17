	file "arith.s"

	global	runerr
	global	ckadd
	global	cksub
	global	ckmul

 text
 even
ckadd:
        link    %a6,&0
        mov.l   8(%a6),%d0      # Perform addition
        add.l   12(%a6),%d0     #  " " "
        #jvs     oflow           # Branch if overflow
	trapv			# This trap untested!!
        unlk    %a6
        rts                     # Return result in d0

 even
cksub:
        link    %a6,&0
        mov.l   8(%a6),%d0      # Perform subtraction
        sub.l   12(%a6),%d0     #  " " "
        #jvs     oflow           # Branch if overflow
	trapv
        unlk    %a6
        rts                     # Return result in d0

 even
ckmul:
        link    %a6,&0
        mov.l   8(%a6),%d0     # Perform multiplication
        muls.w  12(%a6),%d0    #  " " "
	trapv
        #jvs     oflow           # Branch if overflow
        unlk    %a6
        rts                    # Return result in d0

oflow:                         # Got overflow on an operation
        pea     0
        pea     203
        jsr    runerr          # runerr(203,0)
