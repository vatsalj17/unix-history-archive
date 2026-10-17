/ haspmain


/* This information is proprietary and is the property of Bell
/  Telephone Laboratories, Incorporated.  Its reproduction or
/  disclosure to others, either orally or in writing, is pro-
/  hibited without written permission of Bell Laboratories. */


	sleep=35.; kill=37.

start:	mov 2(sp),r0		/pointer to digits
	movb (r0)+,xnow		/current position
	movb (r0)+,xnow+1	/
	movb *r0,xnow+2		/
	mov 4(sp),who		/who=av[1]
	mov 8(sp),inhib		/check status access
	jsr pc,winit		/initialize from who
	sys stat;dot;buf	/stat "."
	mov buf,dev		/save device
	clr nexc		/no exceptions
	jsr pc,getmp		/get temp file
	sys time		/initial time
	mov r1,tout		/
	jsr pc,ckrdy		/check space
loop:	sys stat;hstop;buf	/halt signal?
	jec halt		/yes: halt
	tst rrdy		/ready to recv?
	jeq send		/no: skip msgs
	mov  inhib,r0
	cmpb *r0,$'i		/inhibit console
	jeq  send		/yes
	sys time		/time in r0,r1
	sub tout,r1		/time-out?
	jmi send		/no: try to send
	mov qx,sysq+4		/length
	mov $qx+2,sysq+2	/address
	clr r0			/0=dqs descr
	sys 0;sysq		/querry HASP
	bes 1f			/verify write
	tst r0			/
	bne 3f			/
	mov $ACQ,r0		/can't acquire
	br 2f			/
1:	mov $ERR,r0		/line error
2:	jsr pc,excp		/note exception
	jbr recv		/try later
3:	clr r0			/
	sys write;0;0		/
	bes 1b			/
	clr nexc		/good action
	sys time		/time in r0,r1
	add $600.,r1		/10 minutes
	mov r1,tout		/set time-out
	jbr recv		/response?
send:	mov $2,chks		/2 checks
	tst mesgf		/mesg last?
	bne 1f			/yes: check queue
0:	dec chks		/enough checks?
	jlt recv		/yes: try recv
	tst rrdy		/ready to recv?
	beq 1f			/no: skip msgs
	sys stat;hmesg;buf	/messages?
	bes 1f			/no: check queue
	bit $444,buf+4		/readable?
	beq 1f			/no: skip
	mov $1,mesgf		/flag message
	mov $hmesg,file		/argument file
	br 4f			/join
1:	clr mesgf		/clear flag
	dec chks		/enough checks?
	jlt recv		/yes: try recv
	mov $2,r0		/2=hstat descr
	sys seek;0;0		/set pointer
	sys read;buf;3		/end of queue
	bes 3f			/error: note
	cmp $3,r0		/3 bytes?
	beq 2f			/yes: skip
3:	mov $HST,r0		/hstat error
	jsr pc,excp		/note exception
	jbr recv		/try to receive
2:	cmpb buf,xnow		/queue empty?
	bne 3f			/
	cmpb buf+1,xnow+1	/
	bne 3f			/
	cmpb buf+2,xnow+2	/
	beq 0b			/yes: check mesgs?
3:	mov $xmit,file		/argument file
4:	mov $hxmit,sysx+2	/program
	jsr pc,spawn		/spawn process
	cmp $1,r0		/return code
	bne 5f			/abnormal: decode
	clr nexc		/good action
	tst mesgf		/message?
	jne next		/yes: next
	mov $1,r0		/1=xmit ok
	mov file,r1		/xmitted file
	jsr pc,post		/post event
	jbr next		/next file
0:	.byte ACQ,ERR,OPN,BAD	/send exceptions
5:	cmp $5,r0		/return code
	jlt down		/bad: quit
	movb 0b-2(r0),r0	/exception code
	cmp $ERR,r0		/line error?
	bne 6f			/no: skip
	tst mesgf		/message?
	bne 6f			/yes: skip
	mov r0,-(sp)		/save
	mov $6,r0		/6=line error
	mov file,r1		/file name
	jsr pc,post		/post event
	mov (sp)+,r0		/restore
6:	jsr pc,excp		/note exception
	cmp $ACQ,r0		/can't acquire?
	jeq recv		/yes: try recv
	cmp $ERR,r0		/line error?
	jeq recv		/try recv
	cmp $OPN,r0		/can't open file?
	bne 8f			/no: skip
	tst mesgf		/message?
	bne 7f			/yes: skip
	sys unlink;xmit		/remove file
	mov $xnow,r1		/copy seqno to
	mov $info+9,r2		/ info file name
	movb (r1)+,(r2)+	/
	movb (r1)+,(r2)+	/
	movb (r1)+,(r2)+	/
	sys unlink;info		/delete info file
7:	jbr next		/next
8:	mov $2,r0		/2=bad format
	mov file,r1		/file name
	jsr pc,post		/post event
next:	tst mesgf		/message?
	beq 1f			/no: next file
	sys unlink;hmesg	/remove message
	jbr recv		/response?
1:	mov $xnow,r0		/cushion
	mov $xnow+3,r1		/digit pointer
2:	cmp r0,r1		/3 digits?
	bge 3f			/yes: done
	movb -(r1),r2		/get digit
	inc r2			/inc digit
	movb r2,*r1		/put digit
	cmp $'9,r2		/carry?
	bge 3f			/no: done
	movb $'0,*r1		/zero digit
	br 2b			/carry
3:	jbr loop		/recycle
recv:	jsr pc,ckrdy		/check space
	tst rrdy		/ready to recv?
	beq 5f			/no: delay
	mov nexc,rcnt		/recv  count
0:	clr r0			/0=dqs descr
	sys read;buf;1		/attempt read
	bes 1f			/anything there?
	tst r0			/
	bne 2f			/yes: get it
1:	dec rcnt		/dec count
	bgt 0b			/again?
	tst dne			/wind down
	jeq loop		/no, recycle
	jbr halt2
2:	mov $tmpf,file		/argument file
	mov $hrecv,sysx+2	/program
	jsr pc,spawn		/spawn process
	cmp $4,r0		/return code
	jlt down		/bad: quit
	bne 3f			/recv ok: post
	mov $TMP,r0		/temp trouble
	jsr pc,excp		/note exception
	jsr pc,getmp		/new temp file
	br 2b			/try again
3:	clr nexc		/good action
	add $2,r0		/3-5=recv type
	mov file,r1		/received file
	cmp $5,r0		/5=messages?
	bne 4f			/no: skip
	jsr pc,resp		/cat onto resp file
4:	jsr pc,post		/post event
	jsr pc,getmp		/new temp file
	jsr pc,ckrdy		/check space
	jbr loop		/recycle
5:	mov $SPA,r0		/disk space low
	jsr pc,excp		/note exception
	clr r0			/0=dqs descr
	sys write;0;0		/dummy write
	mov $10.,r0		/10 seconds
	sys sleep		/delay
	jbr loop		/recycle

excp:	mov nexc,r1		/exception count
	movb r0,lexc(r1)	/add to list
	inc r1			/
	mov r1,nexc		/inc count
	cmp $NEXCP,r1		/too many?
	ble 1f			/yes: quit
	cmp $ERR,r0		/line error?
	beq 1f			/yes: quit
	rts pc			/return
1:	cmp $UNX,r0		/system failure?
	beq 2f			/yes: skip
	sys unlink;tmpf		/release temp file
2:	br 1f			/join
down:	mov $UNX,r0		/system failure
	jsr pc,excp		/note exception
1:	mov nexc,sysd+4		/excp count
	mov $2,r0		/hstat descr
	sys seek;0;2		/append
	sys 0;sysd		/excp list
	br 1f			/exit
halt:	sys stat;hsoff;buf	/remote?
	jes halt2
	mov soffo,sysq+4
	mov $soffo+2,sysq+2
	clr r0
	sys 0;sysq		/send signoff message
	clr r0
	sys write;0;0
	mov eot,sysq+4
	mov $eot+2,sysq+2
	clr r0
	sys 0;sysq		/send eot
	clr	r0
	sys	write;0;0
	sys unlink;hsoff
	mov $1,dne		/set wind down flag
	jbr recv		/anything else coming in
halt2:	clr r0			/0=halted
	clr r1			/normally
	jsr pc,post		/post event
	sys unlink;hstop	/remove signal
	sys unlink;tmpf		/release temp file
1:	sys creat;hdead;644	/announce dead
	sys exit		/exit

post:	jsr pc,move		/new name to r1?
	add $'0,r0		/event code '0-6
	movb r0,buf		/
	mov $buf+1,r0		/
	tst r1			/
	beq 2f			/
1:	movb (r1)+,(r0)+	/file name
	bne 1b			/
	movb $'\n,-1(r0)	/lf terminator
2:	sub $buf,r0		/
	mov r0,sysw+4		/length
	mov $1,r0		/1=pipe descr
	sys 0;sysw		/write event
	jes down		/error: quit
	cmp r0,sysw+4		/bytes written
	jne down		/not =: quit
	rts pc			/return

spawn:	sys fork		/fork
	br child		/child return
	bec 1f			/forked: continue
	mov $FRK,r0		/can't fork
	jsr pc,excp		/note exception
	mov $3,r0		/3 seconds
	sys sleep		/delay
	jbr spawn		/try again
1:	mov r0,r5		/subprocess id
	sys wait		/wait subprocess
	jes down		/error: quit
	cmp r0,r5		/correct id?
	beq 2f			/yes: continue
	mov r5,r0		/subprocess id
	sys kill;9		/kill it
	jbr down		/quit
2:	bit $377,r1		/termination
	jne down		/abnormal: quit
	swab r1			/
	bic $!377,r1		/
	mov r1,r0		/return code
	beq 2f			/zero: not run
	rts pc			/return
2:	mov $EXC,r0		/can't execute
	jsr pc,excp		/note exception
	jbr spawn		/try again
child:	mov $1,r0		/1=pipe descr
	sys close		/close
	mov $2,r0		/2=hstat descr
	sys close		/close
	sys 0;sysx		/execute
	clr r0			/can't run
	sys exit		/exit

getmp:	br 1f			/entry point
0:	sys stat;tmpf;buf	/file available?
	bec 1f			/no: try next
	sys creat;tmpf;644	/create it
	jes down		/error: quit
	sys close		/close it
	rts pc			/return
1:	mov $tmpf,r0		/pointer to name
2:	tstb (r0)+		/find end
	bne 2b			/
	dec r0			/
	mov r0,r1		/remember end
3:	movb -(r0),r2		/get digit
	cmp $'9,r2		/'999...?
	blt 4f			/yes: reset
	inc r2			/inc dgit
	movb r2,*r0		/put digit
	cmp $'9,r2		/carry?
	bge 0b			/no: try now
	movb $'0,*r0		/zero digit
	br 3b			/carry
4:	inc r0			/first '9
	movb $'1,(r0)+		/make '1
5:	movb $'0,(r0)+		/append '000...
	cmp r0,r1		/
	ble 5b			/
	br 0b			/try now

winit:	mov $8f,r3		/dest strs
	mov $9f,r2		/init strs
1:	mov (r3)+,r0		/dest
	beq 4f			/done?
	mov who,r1		/prefix
2:	movb (r1)+,(r0)+	/copy
	bne 2b			/
	dec r0			/bsp
3:	movb (r2)+,(r0)+	/copy
	bne 3b			/
	br 1b			/more?
4:	rts pc			/return

8:	hdead;hstop;hmesg;hxmit;hrecv;hsoff;0
9:	<dead\0stop\0mesg\0xmit\0recv\0soff\0>
	.even

.globl _ustat

ckrdy:	mov $buf,-(sp)		/check space
	mov dev,-(sp)		/
	jsr pc,_ustat		/ustat(dev,bf)
	add $4,sp		/
	tst r0			/ustat<0?
	blt 1f			/yes: rrdy
	cmp buf,$1200.		/1200 blocks?
	blo 2f			/no: !rrdy
	cmp buf+2,$50.		/50 inodes?
	blo 2f			/no: !rrdy
1:	mov $1,rrdy		/rrdy=1
	rts pc			/
2:	clr rrdy		/rrdy=0
	rts pc

resp:	mov r0,-(sp)		/save r0 (r1 unchanged)
	mov r1,syso+2		/received file
	clr syso+4		/
	sys 0;syso		/open for read
	bes 5f			/can't: return
	mov r0,r2		/r2=input descr
	sys open;rfile;1	/open resp file
	bec 1f			/ok: continue
	sys creat;rfile;645	/create resp file
	bes 4f			/can't: return
1:	mov r0,r3		/r3=output descr
	sys seek;0;2		/seek to end
2:	mov r2,r0		/input
	sys read;buf;512.	/read block
	bes 3f			/err: break
	tst r0			/eof?
	beq 3f			/yes: break
	mov r0,sysw+4		/byte count
	mov r3,r0		/output
	sys 0;sysw		/write block
	br 2b			/again
3:	mov $'\n,buf		/extra \n
	mov r3,r0		/output
	sys write;buf;1		/write \n
	mov r3,r0		/output
	sys fstat;buf		/stat
	mov r3,r0		/output
	sys close		/close
	tst buf+10.		/>=32K?
	bge 4f			/no: skip
	sys unlink;rfile	/unlink
4:	mov r2,r0		/input
	sys close		/close
5:	mov (sp)+,r0		/restore r0
	rts pc			/return

rfile:	<resp\0>
	.even


move:	cmp $6,r0		/r0=event code
	blo 1f			/out of range: return
	mov r1,r3		/r1=old file
	beq 1f			/0: return
	mov $mbf,r4		/to mbf
3:	movb (r3)+,(r4)+	/copy string
	bne 3b			/
	mov r0,r2		/code
	asl r2			/word index
	mov mv2(r2),r3		/r3=new prefix
	beq 1f			/0: return
	mov $mbf,r4		/to mbf
4:	tstb *r3		/overcopy string
	beq 5f			/
	movb (r3)+,(r4)+	/
	br 4b			/
5:	mov r1,sysl+2		/old file
	mov $mbf,sysl+4		/new file
	sys 0;sysl		/link
	bes 1f			/err: return
	mov r1,sysu+2		/old file
	sys 0;sysu		/unlink
	mov $mbf,r1		/r1=new file
1:	rts pc			/return

mv2:	0;1f;2f;3f;4f;5f;0
1:	<zmit\0>
2:	<badf\0>
3:	<prt\0>
4:	<pch\0>
5:	<msg\0>
	.even


NEXCP	=5			/max exceptions

ACQ	='A			/can't acquire line
BAD	='B			/xmit bad format
ERR	='E			/line error
EXC	='X			/can't execute pgm
FRK	='F			/can't fork process
HST	='S			/hstat read error
OPN	='O			/xmit can't open
SPA	='D			/disk space low
TMP	='T			/temp trouble
UNX	='U			/system failure

dot:	<.\0>			/dot

qx:	29.			/querry ux0;dj1-9999
	.byte 002,141,134,133,344,347,360,100,100,035,177,325,036
	.byte 141,134,133,304,321,361,140,371,371,371,371,035,174,325,036,003
	.even
soffo:	12.			/signoff remote
	.byte 002,141,134,342,311,307,325,326,306,306,036,003
	.even
eot:	1.			/eot
	.byte 067
	.even

.data

sysq:	sys write;0;0		/indirect write
sysw:	sys write;buf;0		/indirect write
sysx:	sys exec;0;file		/indirect execute
sysd:	sys write;lexc;0	/indirect write
file:	0;0			/argument to exec
syso:	sys open;0;0		/indirect open
sysl:	sys link;0;0		/indirect link
sysu:	sys unlink;0		/indirect unlink

xmit:	<xmit>			/xmit queue prefix
xnow:	<000\0>			/queue position
info:	<info/logx000\0>	/log info on xmit
tmpf:	<tmp0\0\0\0\0\0\0\0>	/temporary file

hdead:	<..who..dead\0>		/dead flag
hstop:	<..who..stop\0>		/halt signal
hmesg:	<..who..mesg\0>		/message file
hxmit:	<..who..xmit\0>		/xmitter
hrecv:	<..who..recv\0>		/receiver
hsoff:	<..who..soff\0>		/remote halt signal
	.even

.bss

inhib:	.=.+2			/char *inhib
who:	.=.+2			/char *who
dev:	.=.+2			/device
rrdy:	.=.+2			/ready to recive
dne:	.=.+2
rcnt:	.=.+2			/recv count
tout:	.=.+2			/time-out
mesgf:	.=.+2			/message flag
chks:	.=.+2			/check mesgs,queue
nexc:	.=.+2			/exception count
mbf:	.=.+40.			/buffer for move
buf:	.=.+512.		/stat and block buffer
lexc:	.=.+NEXCP		/exception list

.text

hmain:	mov $0f,r0
	rts pc

0:	<~|^`hmain.s 1.28 4/23/76\0>
	.even

/end
