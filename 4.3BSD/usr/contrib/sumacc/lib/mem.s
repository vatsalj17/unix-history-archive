		.insrt	"../h/sysequ.h"
		.insrt	"../h/toolmacs.h"
		.insrt	"../h/heapdefs.h"

|----------------------------------------------------------------------
|
| procedure setapplbase|
|
| initialize the application zone while growing the system zone.
|
| arguments:
|	a0 - ptr to new application zone base.
|
| registers:
|	d0 - result code from setapplbase (always 0)
|
		.text
		.globl	_setapplb
_setapplb:
|
	movl	sp@+,a1	|return address
	movl	sp@+,a0	|startptr
	.word	__setappllimit	|ask os to do request
	bra	_savereta1	|save 0 result code and return via a1


|----------------------------------------------------------------------
|
| procedure initapplzone|
|
| initialize the application zone
|
| arguments:
|	none
|
| registers:
|	d0 - result code from initapplzone
|
		.text
		.globl	_initappl
_initappl:
|
	.word	__initapplzone	|ask os to do request
	bra	_save


|----------------------------------------------------------------------
|
| procedure initzone(growproc:	procptr|
|	moremasters:	integer|
|	limitptr,startptr : ptr)|
| creates and initializes a fresh zone from unstructured storage
|
| arguments:
|	growproc:	points to grow procedure for this zone
|	moremasters:	number of master pointers to create at a time
|	limitptr:	points one past last byte of raw storage
|	startptr:	points to first byte of raw storage
|
| registers:
|	d0 - result code from initapplzone
|
		.text
		.globl	_initzone
_initzone:
|
	movl	sp@+,a1	|return address
	movl	sp,a0	|stack parameters form req blk
	.word	__initzone
	addw	#14,sp	|strip parameters
	bra	_savereta1	|save 0 result code and return via a1



|----------------------------------------------------------------------
|
| function getzone: thz|
|
| returns value of thezone
|
| arguments:
|	none
|
| result:
|	current value of thezone pointer
|
| registers:
|	a1 -	holds return address
|
		.text
		.globl	_getzone
_getzone:

	.word	__getzone	|ask os to do request
	movl	a0,sp@(4)	|_save result
	bra	_save	|set error code and return


|----------------------------------------------------------------------
|
| procedure setzone(hz: thz)|
|
| sets thezone to hz
|
| arguments:
|	hz:	holds new value for thezone
|
| registers:
|	a2 -	holds return address
|
		.text
		.globl	_setzone
_setzone:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|zone pointer
	.word	__setzone	|ask os to do request
	bra	_savereta1	|save 0 result code and return via a1



|----------------------------------------------------------------------
|
| function appliczone: thz|
|
| returns value of applzone
|
| arguments:
|	none
|
| result:
|	current value of applzone pointer
|
| registers:
|	a1 -	holds return address
|
		.text
		.globl	_appliczo
_appliczo:

	movl	applzone,sp@(4)
	bra	_save0	|set error code 0 and return



|----------------------------------------------------------------------
|
| function systemzone: thz|
|
| returns value of syszone
|
| arguments:
|	none
|
| result:
|	current value of syszone pointer
|
| registers:
|	a1 -	holds return address
|
		.text
		.globl	_systemzo
_systemzo:

	movl	syszone,sp@(4)
	bra	_save0	|set error code and return


|----------------------------------------------------------------------
|
| function compactmem(cbneeded: size): size|
|
| compact the heap until a free block with cbneeded bytes is found or
| until the end of the zone is reached.	returns cbfound, the size of
| the largest block found.
|
| arguments:
|	cbneeded:	size of block needed
|
| result:
|	size of largest block found, in bytes
|
| registers:
|	d0 - number of bytes needed/number of bytes in biggest block
|
		.text
		.globl	_compactm
_compactm:

	movl	sp@+,a1	|get return address
	movl	sp@+,d0	|get cbneeded
	.word	__compactmem	|tell os to do it
	movl	d0,sp@	|_save result
	bra	_save0reta1	|set 0 result code and return via a1


|----------------------------------------------------------------------
|
| procedures _save, save0, save0reta1, savereta1|
|
| _saves d0 into errorcode. alternately sets d0 to 0 first. alternately returns
| via a1.	a1 is destroyed in any case . . .
|
| arguments:
|	d0:	error code to be _saved in errorcode
|
| result:
|	none
|
| registers:
|	a1 - points to errorcode
|
	.globl	errorcode


errorcode:
	.word	0


_save0reta1:
	moveq	#0,d0	|this entry zeroes result, returns via a1

_savereta1:
	movl	a1,sp@-	|a1 has return address

_save:
	movw	d0,errorcode	|_save error

	rts	|return

_save0:
	moveq	#0,d0	|this entry zeroes the result
	bra	_save



|----------------------------------------------------------------------
|
| procedure purgemem(cbneeded: size)|
|
| purge purgeable blocks in the heap until a free block of cbneeded bytes
| is found, or until the end of the zone is reached.
|
| arguments:
|	cbneeded:	size of block needed
|
| result:
|
| registers:
|	a1 - return address
|	d0 - number of bytes needed/error code
|
		.text
		.globl	_purgemem
_purgemem:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,d0	|get cbneeded
	.word	__purgemem	|tell os to do it
	bra	_savereta1	|save result code and return via a1


|----------------------------------------------------------------------
|
| function freemem: longint|
|
| returns number of free bytes in the current zone
|
| arguments:
|	none
|
| result:
|	number of free bytes in the current zone
|
| registers:
|	d0 -	holds result value
|
		.text
		.globl	_freemem
_freemem:

	.word	__freemem	|ask os to do request
	movl	d0,sp@(4)	|_save result
	bra	_save0	|set 0 error code and return



|----------------------------------------------------------------------
|
| procedure resrvmem(cbneeded: size)|
|
| purge purgeable blocks in the heap until a free block of cbneeded bytes
| is found, or until the end of the zone is reached.
|
| arguments:
|	cbneeded:	size of block needed
|
| result:
|
| registers:
|	a1 - return address
|	d0 - number of bytes needed/error code
|
		.text
		.globl	_resrvmem
_resrvmem:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,d0	|get cbneeded
	.word	__resrvmem	|tell os to do it
	bra	_savereta1	|save result code and return via a1




|----------------------------------------------------------------------
|
| function maxmem(var grow: size): size|
|
| return the size of the maximum block which may be allocated without
| growing the zone.	also return the maximum number of bytes by which the
| zone may be grown.
|
| arguments:
|	none
|
| result:
|	size of largest available block in bytes
|	grow:	maximum growth allowed in current zone
|
| registers:
|	a0 - grow/pointer to var grow
|	a1 - return address
|	d0 - number of bytes in largest free block
|
		.text
		.globl	_maxmem
_maxmem:


	movl	sp@+,a1	|get return address
	.word	__maxmem	|let os do it
	movl	a0,d1	|max growth allowed
	movl	sp@+,a0	|get the pointer to var grow
	movl	d1,a0@
	movl	d0,sp@	|_save result
	bra	_save0reta1	|set 0 result code and return via a1



|----------------------------------------------------------------------
|
| function topmem: ptr|
|
| returns the address of the byte after the last byte of real memory.
|
| arguments:
|	none
|
| result:
|	ptr:	address of byte just following last byte of
|	real memory
|
| registers:
|	d0 - result code
|
		.text
		.globl	_topmem
_topmem:

	movl	memtop,sp@(4)	|set function result
	bra	_save0	|set result and return



|----------------------------------------------------------------------
|
| procedure setgrowzone(growzone: procptr)|
|
| sets the current heap zone's growzone procedure to growzone
|
| arguments:
|	growzone:	new value for this zone's growzone proc
|
| registers:
|	a1 -	holds return address
|
		.text
		.globl	_setgrowz
_setgrowz:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|procedure pointer
	.word	__setgrowzone	|ask os to do request
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| procedure setappllimit(zonelimit: ptr)|
|
| sets the application zone's limit to zonelimit
|
| arguments:
|	zonelimit:	new value for application zone limit
|
| registers:
|	a2 -	holds return address
|
		.text
		.globl	_setappll
_setappll:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|zone pointer
	.word	__setappllimit	|ask os to do request
	bra	_savereta1	|save 0 result code and return via a1


|procedure maxapplzone;
|
|grows the application zone to as large as allowable (by appllimit)

		.text
		.globl	_maxapplz
_maxapplz:

	movl	appllimit,a0
	lea	heapend,a1
	movl	a0,d0
	subl	a1@,d0	|bytes to grow
	moveq	#minfree,d2
	cmpl	d2,d0
	bcss	1$	|br if no room to grow

	movl	a1@,a1	|point to current limit blk
	movl	a0,heapend	|set heapend=appllimit
	movl	d0,a1@	|inc size of current limit blk
	clrb	a1@	|just to be sure

	movl	applzone,a1	|adjust heap zone header
	movl	a0,a1@	|new limit block (bklim)
	movl	d2,a0@	|limit block size is always min
	addl	d0,a1@(zcbfree)	|increase free count

1$:	rts

|----------------------------------------------------------------------
|
| function newptr(bytecount: size): ptr|
|
| returns a pointer to a newly allocated non-relocatable block of
| memory bytecount bytes long.
|
| arguments:
|	bytecount:	number of bytes needed
|
| result:
|	pointer to new block, or nil, if not enough room
|
| registers:
|	a0 - points to new block, or nil
|	a1 - return address
|	d0 - number of bytes needed/error code
|
		.text
		.globl	_newptr
_newptr:

	movl	sp@+,a1	|get return address
	movl	sp@+,d0	|get the byte count
	.word	__newptr	|ask os to do request
	movl	a0,sp@	|return result ptr on stack
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| procedure disposptr(p: ptr)|
|
| releases the block pointed to by p.
|
| arguments:
|	p:	pointer to block to be freed
|
| result:
|	none
|
| registers:
|	a0 - points to block to be freed
|	a1 - return address
|	d0 - error code
|
		.text
		.globl	_dispospt
_dispospt:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get parameter
	.word	__disposptr	|let os do work
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| function getptrsize(p: ptr): size|
|
| returns number of bytes in the non-relocatable block pointed to by p.
|
| arguments:
|	p:	points to non-relocatable block
|
| result:
|	number of bytes in block
|
| registers:
|	a0 -	points to block
|	a1 -	holds return address
|	d0 -	number of bytes in block
|
		.text
		.globl	_getptrsi
_getptrsi:

	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get the pointer
	.word	__getptrsize	|let os do it
	movl	d0,sp@	|return result on stack
	bpl	.L1	|true sizes are positive
	clrl	sp@	|return 0 on errors
	bra	_savereta1	|and set error code

.L1:	bra	_save0reta1	|set 0 result code and return via a1


|----------------------------------------------------------------------
|
| procedure setptrsize(p: ptr| newsize: size)|
|
| sets the size of the non-relocatable block pointed to by p to newsize
| bytes.
|
| arguments:
|	p:	points to non-relocatable block
|	newsize:	number of bytes needed in block
|
| result:
|	none
|
| registers:
|	a0 -	points to block
|	a1 -	holds return address
|	d0 -	number of bytes needed/error code
|
		.text
		.globl	_setptrsi
_setptrsi:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,d0	|get the new size
	movl	sp@+,a0	|get the pointer
	.word	__setptrsize	|let os do it
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| function	ptrzone(p: ptr): thz|
|
| recovers the reference to the heap zone object, given a pointer to
| a block.
|
| arguments:
|	p:	points to non-relocatable block
|
| result:
|	pointer to the zone object for the zone
|	containing the referenced block.
|
| registers:
|	a0 -	points to block/points to block's zone object
|	a1 -	holds return address
|	d0 -	error code
|
		.text
		.globl	_ptrzone
_ptrzone:

	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get the pointer
	.word	__ptrzone	|let os do it
	movl	a0,sp@	|_save zone object pointer
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| function	newhandle(bytecount: size): handle|
|
| returns a handle to a newly allocated relocatable block
| bytecount bytes long.
|
| arguments:
|	bytecount:	number of bytes needed
|
| result:
|	handle to new block, or nil, if not enough room
|
| registers:
|	a0 - handle for new block, or nil
|	a1 - return address
|	d0 - number of bytes needed/error code
|
		.text
		.globl	_newhandl
_newhandl:

	movl	sp@+,a1	|get return address
	movl	sp@+,d0	|get the byte count
	.word	__newhandle	|ask os to do request
	movl	a0,sp@	|return result handle on stack
	bra	_savereta1	|save result code and return via a1


|----------------------------------------------------------------------
|
| procedure disposhandle(h: handle)|
|
| releases the block referenced by h.
|
| arguments:
|	h:	handle for the block to be freed
|
| result:
|	none
|
| registers:
|	a0 - handle for block to be freed
|	a1 - return address
|	d0 - error code
|
		.text
		.globl	_disposha
_disposha:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get handle
	.word	__disposhandle	|let os do work
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| function	gethandlesize(h: handle): size|
|
| returns number of bytes in the relocatable block referenced by h.
|
| arguments:
|	h:	handle for relocatable block
|
| result:
|	number of bytes in block
|
| registers:
|	a0 -	points to block
|	a1 -	holds return address
|	d0 -	number of bytes in block
|
		.text
		.globl	_gethandl
_gethandl:

	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get the handle
	.word	__gethandlesize	|let os do it
	movl	d0,sp@	|return result on stack
	bpl	.L2	|real sizes are positive
	clrl	sp@	|on errors, return 0
	bra	_savereta1	|and set error code correctly

.L2:	bra	_save0reta1	|if result is a size, set 0 result code


|----------------------------------------------------------------------
|
| procedure sethandlesize(h: handle| newsize: size)|
|
| sets the size of the relocatable block referred to by h to newsize
| bytes.
|
| arguments:
|	h:	refers to relocatable block
|	newsize:	number of bytes needed in block
|
| result:
|	none
|
| registers:
|	a0 -	points to block
|	a1 -	holds return address
|	d0 -	number of bytes needed/error code
|
		.text
		.globl	_sethandl
_sethandl:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,d0	|get the new size
	movl	sp@+,a0	|get the handle
	.word	__sethandlesize	|let os do it
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| function	handlezone(h: handle): thz|
|
| recovers the reference to the heap zone object, given a handle for
| a block.
|
| arguments:
|	h:	refers to relocatable block
|
| result:
|	pointer to the zone object for the zone
|	containing the referenced block.
|
| registers:
|	a0 -	handle for block/points to block's zone object
|	a1 -	holds return address
|	d0 -	error code
|
		.text
		.globl	_handlezo
_handlezo:

	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get the handle
	.word	__handlezone	|let os do it
	movl	a0,sp@	|_save zone object pointer
	bra	_savereta1	|save result code and return via a1


|----------------------------------------------------------------------
|
| function	recoverhandle(p: ptr): handle|
|
| recovers the handle for a relocatable block, given a pointer to the
| relocatable block.	thezone must be set to reflect the zone containing
| this relocatable block.
|
| arguments:
|	rp:	points to relocatable block
|
| result:
|	handle for the referenced block.
|
| registers:
|	a0 -	pointer to relocatable block/handle for block
|	a1 -	holds return address
|	d0 -	error code
|
		.text
		.globl	_recoverh
_recoverh:

	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get the reloc block pointer
	.word	__recoverhandle	|let os do it
	movl	a0,sp@	|_save handle for block
	bra	_save0reta1	|save result code and return via a1
	|(os fails to set d0 for this call so
	| we unconditionally set result to 0)


|----------------------------------------------------------------------
|
| procedure emptyhandle(h: handle)|
|
| explicitly purges the relocatable block referred to by h.
|
| arguments:
|	h:	refers to relocatable block
|
| result:
|	none
|
| registers:
|	a0 -	handle for block
|	a1 -	holds return address
|	d0 -	error code
|
		.text
		.globl	_emptyhan
_emptyhan:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,a0	|get the handle
	.word	__emptyhandle	|let os do it
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| procedure reallochandle(h: handle| bytecount: size)|
|
| allocates a relocatable block, using an existing handle.
| if the handle is not already empty, the handle is emptied first,
| then allocated.
|
| arguments:
|	h:	refers to relocatable block
|	bytecount:	number of bytes needed in the new block
|
| result:
|	none
|
| registers:
|	a0 -	handle for block/points to block's zone object
|	a1 -	holds return address
|	d0 -	new block size/error code
|
		.text
		.globl	_realloch
_realloch:
|
	movl	sp@+,a1	|get return address
	movl	sp@+,d0	|get the new size
	movl	sp@+,a0	|get the handle
	.word	__reallochandle	|let os do it
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| procedure hlock(h: handle)|
| procedure hunlock(h: handle)|
| procedure hpurge(h: handle)|
| procedure hnopurge(h: handle)|
|
| these procedure are used to change the lock and purge attributes of
| the relocatable block referred to by h.
|
| arguments:
|	h:	handle for relocatable block
|
| registers:
|	a0 -	handle for relocatable block
|	a1 -	holds return address
|
		.text
		.globl	_hlock
_hlock:
|
	movl	sp@+,a1	|return address
	movl	sp@+,a0	|handle for block
	.word	__hlock	|let os to do it.
	bra	_savereta1	|save result code and return via a1

		.text
		.globl	_hunlock
_hunlock:
|
	movl	sp@+,a1	|return address
	movl	sp@+,a0	|handle for block
	.word	__hunlock	|let os to do it.
	bra	_savereta1	|save result code and return via a1

		.text
		.globl	_hpurge
_hpurge:
|
	movl	sp@+,a1	|return address
	movl	sp@+,a0	|handle for block
	.word	__hpurge	|let os to do it.
	bra	_savereta1	|save result code and return via a1

		.text
		.globl	_hnopurge
_hnopurge:
|
	movl	sp@+,a1	|return address
	movl	sp@+,a0	|handle for block
	.word	__hnopurge	|let os to do it.
	bra	_savereta1	|save result code and return via a1


|
|procedure moremasters|
|

		.text
		.globl	_moremast
_moremast:

	.word	__moremasters
	bra	_save


|----------------------------------------------------------------------
|
| procedure blockmove(srcptr, destptr: ptr| bytecount: size)|
|
| marks the relocatable block referred to by h as not purgeable.
|
| arguments:
|	srcptr:	source pointer
|	destptr:	destination pointer
|	bytecount:	bytecount for move
|
| registers:
|	a0 -	source pointer
|	a1 -	destination pointer
|	d0 -	bytecount for move
|	d1 -	holds return address
|
		.text
		.globl	_blockmov
_blockmov:
|
	movl	sp@+,d1	|return address
	movl	sp@+,d0	|byte count
	movl	sp@+,a1	|destination ptr
	movl	sp@+,a0	|source ptr
	.word	__blockmove	|let os to do it.
	movl	d1,a1	|return address
	bra	_savereta1	|save result code and return via a1



|----------------------------------------------------------------------
|
| function memerror: oserr|
|
| returns the error observed by the most recent memory manager call.
|
| arguments:
|	none
|
| registers:
|
		.text
		.globl	_memerror
_memerror:
	.globl	errorcode

	movw	errorcode,sp@(4) |last error
	rts


|----------------------------------------------------------------------
|
| function	gzcritical: boolean|
| function	gz_savehnd: handle|
|
| these routines are provided to help pascal growzone functions make
| decisions.
|
| arguments:
|	none
|
| registers:
|
		.text
		.globl	_gz_saveh
_gz_saveh:

	movl	gzroothnd,sp@(4) |handle which should not be touched
	rts

		.text
		.globl	_gzcritic
_gzcritic:
	movl	sp@+,a1	|return address
	clrw	sp@	|assume non-critical
	movl	gzmovehnd,d0
	beq	critcase
	cmpl	gzroothnd,d0
	bne	noncrit
critcase:
	movw	#/0100,sp@	|return true

noncrit:	jmp	a1@

