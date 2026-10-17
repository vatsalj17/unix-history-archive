	.title	ux.sml


;***************************

.macro	$param
	r0=%0
	r1=%1
	r2=%2
	r3=%3
	r4=%4
	r5=%5
	sp=%6
	pc=%7
.endm	$param

;***************************

.macro	$indir
	104400 +	0
.endm	$indir

;***************************

.macro	$exit
	104400 +	1
.endm	$exit

;***************************

.macro	$fork
	104400 +	2
.endm	$fork

;***************************

.macro	$read
	104400 +	3
.endm	$read

;***************************

.macro	$write
	104400 +	4
.endm	$write

;***************************

.macro	$open
	104400 +	5
.endm	$open

;***************************

.macro	$close
	104400 +	6
.endm	$close

;***************************

.macro	$wait
	104400 +	7
.endm	#wait

;***************************

.macro	$creat
	104400 +	8.
.endm	$creat


;***************************

.macro	$link
	104400 +	9.
.endm	$link


;***************************

.macro	$unlink
	104400 +	10.
.endm	$unlink

;******************************

.macro	$exec
	104400 +	11.
.endm	$exec

;***************************

.macro	$chdir
	104400 +	12.
.endm	$chdir


;****************************

.macro	$time
	104400 +	13.
.endm	$time

;****************************

.macro	$mknod
	104400 +	14.
.endm	$mknod

;****************************

.macro	$chmod
	104400 +	15.
.endm	$chmod

;***************************

.macro	$chown
	104400 +	16.
.endm	$chown

;***************************

.macro	$break
	104400 +	17.
.endm	$break

;***************************

.macro	$stat
	104400 +	18.
.endm	$stat

;***************************

.macro	$seek
	104400 +	19.
.endm	$seek

;*************************

.macro $getpid
	104400 +	20.
.endm

;******************************

.macro	$mount
	104400 +	21.
.endm	$mount

;*****************************

.macro	$umount
	104400 +	22.
.endm	$umount

;****************************

.macro	$setuid
	104400 +	23.
.endm	$setuid


.macro	$setruid
	104400 +	23.
.endm	$setruid

;****************************

.macro	$getruid
	104400 +	24.
.endm



.macro	$getuid
	104400 +	24.
.endm

;***************************

.macro	$stime
	104400 +	25.
.endm	$stime

;*******************************
.macro $ptrace
	104400 + 	26.
.endm

;******************************

.macro	$fstat
	104400 +	28.
.endm	$fstat

;*****************************

.macro	$smdate
	104400 +	30.
.endm	$smdate

;*****************************

.macro	$stty
	104400 +	31.
.endm	$stty

;****************************

.macro	$gtty
	104400 +	32.
.endm	$gtty

;****************************

.macro	$nice
	104400 +	34.
.endm	$nice

;****************************

.macro	$sleep
	104400 +	35.
.endm	$sleep

;*****************************

.macro	$sync
	104400 +	36.
.endm	$sync

;*****************************

.macro	$kill
	104400 +	37.
.endm	$kill

;*****************************

.macro	$switch
	104400 +	38.
.endm	$switch

;****************************

.macro	$dup
	104400 +	41.
.endm	$dup

;*****************************

.macro	$pipe
	104400 +	42.
.endm	$pipe

;*****************************

.macro	$times
	104400 +	43.
.endm	$times

;****************************

.macro	$prof
	104400 +	44.
.endm	$prof

.macro $profile
	104400 +	44.
.endm

;*****************************

.macro	$seteuid
;.macro	$setgid
	104400 +	46.
.endm

;****************************

.macro	$geteuid
;.macro	$getgid
	104400 +	47.
.endm

;****************************

.macro	$sig
	104400 +	48.
.endm	$sig

;******************************


.end


