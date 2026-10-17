	.insrt	"h.h"

| interface for file system and io procedures and functions which do not require
| a parameter block
|
| procedure adddrive(drvrrefnum: integer| drvnum: integer| qel: drvqelptr)|

	.text
	.globl	_adddrive
_adddrive:
	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|ptr to memory for queue element
	movw	sp@+,d0	|drive number
	swap	d0	|put it in high word
	movw	sp@+,d0	|driver ref num
	.word	__adddrive
	jmp	a1@
