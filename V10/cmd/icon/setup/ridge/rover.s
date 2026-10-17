        dsect data
        csect code
        align 2
	global	_runerr
	global	_ckadd
	global	_cksub
	global	_ckmul


_ckadd: 
        load    r0,r14,24       ; r0 gets first argument
        load    r1,r14,32       ; r1 gets second argument
        add     r0,r1           ; Perform addition
;br     oflow,L         ; Branch if overflow
        ret     r11,r11         ; Return result in r0

        dsect data
        csect code
        align   2
_cksub:
        load    r0,r14,24       ; r0 gets first argument
        load    r1,r14,32       ; r1 gets second argument
        sub     r0,r1           ; Perform subtraction
;br     oflow,L         ; Branch if overflow
        ret     r11,r11         ; Return result in r0
        
        dsect data
        csect code
        align   2
_ckmul:
        load    r0,r14,24       ; r0 gets first argument
        load    r1,r14,32       ; r1 gets second argument
        mpy     r0,r1           ; Perform multiplication
;br     oflow           ; Branch if overflow
        ret     r11,r11         ; Return result in r0

oflow:                          ; Got overflow on an operation
        laddr   r14,r14,-40
        movei   r0,0
        store   r0,r14,32
        laddr   r0,23
        store   r0,r14,24
        call    r11,_runerr,L   ; runerr(203,0)

