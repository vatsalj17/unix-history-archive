/* send9.s */


/* This information is proprietary and is the property of Bell
/  Telephone Laboratories, Incorporated.  Its reproduction or
/  disclosure to others, either orally or in writing, is pro-
/  hibited without written permission of Bell Laboratories. */


.globl _send9

_send9:	mov $id,r0
	rts pc

.data

id:	<~|^`send9.s 1.5 1/15/76\0>
	.even

.text

/ struct trap
/   {struct trap *str;
/    int ssp,sr5,spc;} trp;

/ setrp(&trp) stacks trap
/ rstrp(&trp) unstacks trap
/ dotrp() performs last trap

.globl _setrp,_rstrp,_dotrp

_setrp:	mov 2(sp),r0
	mov 0f,(r0)+
	mov sp,(r0)+
	mov r5,(r0)+
	mov (sp),(r0)+
	mov 2(sp),0f
	rts pc

_rstrp:	mov *2(sp),0f
	rts pc

_dotrp:	mov 0f,r0
	beq 1f
	tst (r0)+
	mov (r0)+,sp
	mov (r0)+,r5
	mov (r0)+,(sp)
1:	rts pc

.bss

0:	.=.+2

/end
