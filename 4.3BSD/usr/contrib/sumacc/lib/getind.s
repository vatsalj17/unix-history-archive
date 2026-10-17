	.insrt	"../h/toolmacs.h"

|procedure getindstring(var thestring: str255; strlistid: integer;
|	index: integer);
|
|gets the str# resource with id strlistid andw returns in thestring the index-th
|string in the string list.

		.text
		.globl	_getindst
_getindst:

	link	a6,#0

	|get the string list
	subqw	#4,sp	|room for result handle
	|movl	#'STR#',sp@-	|thetype
	movl	.L1,sp@-	|thetype
	movw	a6@(10),sp@-	|strlistid
	.word	__getresou
	movl	a6@(12),a1	|ptr to where to return the str
	clrb	a1@	|assume empty
	movl	sp@+,d0	|handle to string list
	beqs	gsret	|if empty, just quit
	movl	d0,a0	|get handle in a0
	movl	a0@,a0	|deref it
	movw	a0@+,d0	|get the number of strings
	movw	a6@(8),d1	|the index
	cmpw	d0,d1	|is index larger than count?
	bgts	gsret	|yes, just quit
	moveq	#0,d0	|will keep track of length of strs

gsloop:	subqw	#1,d1	|decrement the index
	beqs	gscopy	|found the one we are looking for
	movb	a0@+,d0	|get length of this one
	addl	d0,a0	|skip over this string
	bras	gsloop	|and go on to next

gscopy:	movb	a0@,d0	|get the length of this string
	addqw	#1,d0	|add one for the length byte
	.word	__blockmove	|copy it into the string

gsret:	unlk	a6
	movl	sp@+,a0	|return addr
	addql	#8,sp	|pop off parms
	jmp	a0@	|return

.L1:	.ascii	"STR#"


|procedure getindpattern(var thepat: pattern; patlistid: integer;index: integer)
|
|gets pat# resource with id patlistid andw stuffs into thepat the index-th
|pattern in the list.

		.text
		.globl	_getindpa
_getindpa:

	link	a6,#0

	|get the string list
	subqw	#4,sp	|room for result handle
	|movl	#'PAT#',sp@-	|thetype
	movl	.L2,sp@-	|thetype
	movw	a6@(10),sp@-	|patlistid
	.word	__getresou
	movl	a6@(12),a1	|ptr to where to return the pat
	movl	sp@+,d0	|handle to string list
	beqs	gpret	|if empty, just quit
	movl	d0,a0	|get handle in a0
	movl	a0@,a0	|deref it
	movw	a0@+,d0	|get the number of patterns
	movw	a6@(8),d1	|the index
	bles	gpret	|no such thing as negative index
	cmpw	d0,d1	|is index larger than count?
	bgts	gpret	|yes, just quit

gploop:	subqw	#1,d1	|decrement the index
	beqs	gpcopy	|found the one we are looking for
	addqw	#8,a0	|skip over the pattern
	bras	gploop	|and go on to next

gpcopy:	moveq	#8,d0	|a pattern is 8 bytes
	.word	__blockmove	|copy it into the string

gpret:	unlk	a6
	movl	sp@+,a0	|return addr
	addql	#8,sp	|pop off parms
	jmp	a0@	|return

.L2:	.ascii	"PAT#"
