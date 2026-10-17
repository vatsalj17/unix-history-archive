/ rjestat7.s


/* This information is proprietary and is the property of Bell
/* Telephone Laboratories, Incorporated.  Its reproduction or
/* disclosure to others, either orally or in writing, is pro-
/* hibited without written permission of Bell Laboratories. */


.globl _rstat7

_rstat7: mov $id,r0
	rts pc

.data

id:	<~|^`rjestat7.s 1.3 1/29/76\0>
	.even

.text

/ del2(t0,t1)
/  int t0[2],t1[2];
/
/  single-precision signed result of t1-t0

.globl _del2

_del2:	mov r2,-(sp)
	mov 4(sp),r2
	mov (r2)+,r1
	mov (r2),r0
	com r0
	com r1
	add $1,r0
	adc r1
	mov 6(sp),r2
	add 2(r2),r0
	adc r1
	add (r2),r1
	mov $77777,r2
	tst r1
	blt 2f
	bne 1f
	cmp r2,r0
	bhis 4f
1:	mov r2,r0
	br 4f
2:	cmp $-1,r1
	bne 3f
	cmp r2,r0
	blo 4f
3:	mov $100000,r0
4:	mov (sp)+,r2
	rts pc

/end
