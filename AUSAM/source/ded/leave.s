/  leave and recall
/  last modified 4/5/77
/  requires text and data segments in the same address space
/  calls nargs, abort

/ notice how recall works ... it won't if the function being
/ recalled is an assembler function which does not start with
/ a jsr r5, csv. It would be possible to get r2-r4 off the stack
/ and then do a jmp to the first instruction of the function;
/ however the heap system uses the stack locations for r2 and r3
/ for its own purposes (they are stored elsewhere)

	.globl	_recall, _leave, csv, cret
	.text

_leave:	
	jsr	r5, csv
	mov	6(r5), r0
	jsr	pc, find	/ search stack
	mov	r4, r5
	jmp	cret		/ return


_recall:
	jsr	r5, csv
	jsr	pc, _nargs
	dec	r0
	mov	r0, -(sp)
	jsr	pc, find
	mov	r5, r2		/ save recall's level
	mov	r4, r5		/ set new level
	jsr	pc, _nargs	/ how many args has function got
	cmp	r0, (sp)	/ does recall have too many args
	jge	ok
	mov	r2, r5		/ restore stack
	jsr	pc, _abort
ok:	mov	r5, r1
	add	$4, r1		/ base of argument vector, recalled function
	add	$6, r2		/ base of argument vector in recall
	mov	(sp), r0	/ only copy defined args from recall
copy:	jeq	enter
	mov	(r2)+, (r1)+	/ copy recall args to function
	dec	r0
	jbr	copy

enter:	add	$-10, r4
	mov	r4, sp
	jmp	4(r3)		/ enter just after csv sequence






/  r5 address of stack thread
/  clobbers r1
/  exits with r4 pointing to required activation
/  and        r3 address of function found
/  otherwise calls abort

find:	mov	4(r5), r3	/ address of function to be found on stack
	mov	r5, r4
loop:	mov	2(r4), r1	/ find activation for function
	cmp	$4737, -4(r1)	/ call from jsr pc, *$rout?
	jne	1f
	cmp	r3, -2(r1)
	jne	1f
	rts	pc
1:	cmp	$4767, -4(r1)	/ call from jsr pc, rout?
	jne	nextsp
	add	-2(r1), r1
	cmp	r1, r3
	jne	nextsp
	rts	pc


nextsp:	mov	(r4), r4	/ follow stack
	jne	loop
	jsr	pc, _abort	/ function not found

