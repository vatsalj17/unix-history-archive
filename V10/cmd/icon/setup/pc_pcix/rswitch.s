.globl _reflag
.globl _coswitch
.globl _boundary
.globl _sp
.globl __brkend
.globl csv
.globl cret

			| coswitch(oldstate, newstate, first)
			| int *oldstate, *newstate;
			| int first;
_coswitch:
			| save machine state on stack
	push bp
	mov bp,sp
	push di
	push si
	push _boundary
	mov bx,*4(bp)
	mov (bx),sp
	mov bx,*6(bp)
	mov ax,*8(bp)
	or ax,ax
	jnz not_first
			| if (first == 0) set up new stack
	lea di,_sp
	mov _boundary,di
	mov ax,(bx)
	mov sp,ax
	sub ax,ax
	push ax
	push ax
	call _interp
	pop ax		| clear args from stack
	pop ax
	mov ax,#badret
	call _syserr

not_first:
	mov ax,(bx)	| switch to new stack
	mov sp,ax

	pop _boundary	| and restore machine state from stack
	pop si
	pop di
	pop bp
	ret

.data
badret:
	.asciz "interp() returned in coswitch"

	.text
	.globl _xruner
	.globl _runerr
	.globl _reflag
	.globl _sp
	.globl _abort

			| runerr turns off stack probes and calls xrunerr
			|  without this, calls to runerr might recurse
			|  infinitely 
_runerr:
	mov _reflag,#1		| disable stack probes
	jmp _xruner

_reflag:	.word *0	| setting reflag disables stack probes
_boundary:	.word __brkend	| address of var representing limit on stack
				|  growth

	.text

		| csv - save registers and set up procedure frame
		|  verify that stack space is sufficient for interrupt
		|  handlers and routine arithmetic	
csv:
~csv=.
	pop	bx		| bx gets return address
	push	bp
	mov	bp,sp
	push	di
	push	si

	sub sp,ax		
	cmp _reflag,#1
	je csv_ok		| stack probes disabled
	mov ax,sp
	mov di,_boundary
	sub ax,(di)
	sub ax,#128
	ja csv_ok
	
	mov ax,#0
	push ax
	mov ax,#302
	push ax
	call _runerr
	call _abort		| runerr shouldn't return

csv_ok:
	jmp @bx

			| cret - restore saved registers and remove 
			|  procedure frame
cret:
~cret=.
	lea	sp,*-4(bp)
	pop	si
	pop	di
	pop 	bp
	ret


	.data

