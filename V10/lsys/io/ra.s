.data
.globl _rabdev
.align 2; _rabdev:.long _raopen
.long _raclose
.long _rastrategy
.long 0
.globl _racdev
.align 2; _racdev:.long _raopen
.long _raclose
.long _raread
.long _rawrite
.long _raioctl
.long _nulldev
.long 0x0
.globl _ra_sizes
.align 2; _ra_sizes:.long 10240
.long 0
.long 20480
.long 10240
.long 249848
.long 30720
.long 249848
.long 280568
.long 249848
.long 530416
.long 2147483647
.long 780264
.long 749544
.long 30720
.long 2147483647
.long 0
.text
.globl _raopen
.align 1
_raopen:.word 0xe00
movw 4(ap),4(ap)
movzwl 4(ap),r5
ashl $-3,r5,r5
bicl3 $0xfffffff8,r5,r11
cmpl r11,_racnt; jlss L10
movb $6,_u+197
ret
L10:mull3 r11,$44,r5
addl3 $_radisk,r5,r10
moval 0[r11],r5
addl3 $_raaddr,r5,r9
tstw 4(r10); jneq L12
cvtbl 1(r9),r5
jlss L17
cmpl r5,_nmsport; jgeq L17
movl _msportsw[r5],(r10)
jnequ L14
L17:movb $6,_u+197
ret
L14:movl $2147483647,24(r10)
pushal _radg
pushal _raseql
clrl -(sp)
clrl -(sp)
cvtbl 1(r9),-(sp)
cvtbl (r9),-(sp)
movl *(r10),r5
calls $24/4,(r5)
tstl r0; jneq L18
movb $6,_u+197
ret
L18:pushl r9
pushl r10
calls $8/4,_racinit
L12:movzwl 4(ap),r5
bicl2 $0xfffffff8,r5
ashl r5,$1,r5
bisw2 r5,4(r10)
calls $0/4,_spl6
cvtbl 6(r10),r5
bicl2 $0xfffffffe,r5
tstl r5; jneq L20
pushl r9
pushl r10
calls $8/4,_raonline
L20:calls $0/4,_spl0
cvtbl 6(r10),r5
bicl2 $0xfffffffe,r5
tstl r5; jneq L22
movb $6,_u+197
L22:ret
.globl _raclose
.align 1
_raclose:.word 0xe00
movl 4(ap),r5
ashl $-3,r5,r4
bicl2 $0xfffffff8,r4
mull3 r4,$44,r3
addl3 $_radisk,r3,r11
moval 0[r4],r4
addl3 $_raaddr,r4,r10
bicl2 $0xfffffff8,r5
ashl r5,$1,r5
mcoml r5,r5
mcoml r5,r5
bicw2 r5,4(r11)
tstw 4(r11); jneq L26
cvtbl 6(r11),r5
bicl2 $0xfffffffe,r5
tstl r5; jneq L24
L26:ret
L24:cvtbl (r10),-(sp)
movl (r11),r5
calls $4/4,*4(r5)
movl r0,r9
addl3 $1,_rarefno,r5
movl r5,_rarefno
movl r5,(r9)
movw 2(r10),4(r9)
movb $8,8(r9)
cvtbl 6(r11),r5
bicl2 $0xffffffbf,r5
tstl r5; jneq L27
clrw 10(r9)
jbr L28
L27:movw $1,10(r9)
bicb2 $64,6(r11)
L28:clrw 14(r9)
clrl 28(r9)
bicb2 $1,6(r11)
pushl r9
clrl -(sp)
cvtbl (r10),-(sp)
movl (r11),r5
calls $12/4,*12(r5)
ret
.globl _raread
.align 1
_raread:.word 0x0
pushal _minphys
pushl $1
movl 4(ap),r5
pushl r5
ashl $-3,r5,r5
bicl2 $0xfffffff8,r5
mull2 $44,r5
pushal _rabuf(r5)
pushal _rastrategy
calls $20/4,_physio
ret
.globl _rawrite
.align 1
_rawrite:.word 0x0
pushal _minphys
clrl -(sp)
movl 4(ap),r5
pushl r5
ashl $-3,r5,r5
bicl2 $0xfffffff8,r5
mull2 $44,r5
pushal _rabuf(r5)
pushal _rastrategy
calls $20/4,_physio
ret
.globl _rastrategy
.align 1
_rastrategy:.word 0xfc0
subl2 $12,sp
movl 4(ap),r11
movzwl 26(r11),r5
bicl2 $0xffffff00,r5
ashl $-3,r5,r5
bicl3 $0xfffffff8,r5,r8
mull3 r8,$44,r5
addl3 $_radisk,r5,r10
moval 0[r8],r5
addl3 $_raaddr,r5,r6
movzwl 26(r11),r5
bicl2 $0xffffff00,r5
bicl2 $0xfffffff8,r5
movad 0[r5],r5
addl3 $_ra_sizes,r5,r7
subl3 4(r7),24(r10),-8(fp)
cmpl -8(fp),(r7); jleq L29
movl (r7),-8(fp)
L29:cmpl 32(r11),-8(fp); jlss L31
cmpl r11,$_rctbuf; jeqlu L31
cmpl 32(r11),(r7); jneq L33
movl 20(r11),36(r11)
jbr L34
L33:movw $28,24(r11)
bisl2 $4,(r11)
L34:pushl r11
calls $4/4,_iodone
ret
L31:movl 20(r11),-4(fp)
divl3 $512,-4(fp),r5
addl2 32(r11),r5
cmpl r5,-8(fp); jleq L35
cmpl r11,$_rctbuf; jeqlu L35
subl3 32(r11),-8(fp),r5
ashl $9,r5,-4(fp)
L35:calls $0/4,_spl6
cvtbl 6(r10),r5
bicl2 $0xfffffffe,r5
tstl r5; jneq L37
pushl r6
pushl r10
calls $8/4,_raonline
tstl r0; jneq L37
bisl2 $4,(r11)
pushl r11
calls $4/4,_iodone
calls $0/4,_spl0
ret
L37:cvtbl (r6),-(sp)
movl (r10),r5
calls $4/4,*4(r5)
movl r0,r9
addl3 $1,_rarefno,r5
movl r5,_rarefno
movl r5,(r9)
movw 2(r6),4(r9)
bicl3 $0xfffffffe,(r11),r5
jeql L40
movl $33,-12(fp)
jbr L41
L40:movl $34,-12(fp)
 # label
L41:movb -12(fp),8(r9)
clrw 10(r9)
movl -4(fp),12(r9)
addl3 4(r7),32(r11),28(r9)
pushl r11
pushl r9
cvtbl (r6),-(sp)
movl (r10),r5
calls $12/4,*8(r5)
movl r9,16(r11)
movl (r9),36(r11)
clrl 12(r11)
tstl 16(r10); jeqlu L42
movl 20(r10),r5
movl r11,12(r5)
jbr L43
L42:movl r11,16(r10)
L43:movl r11,20(r10)
pushl r9
clrl -(sp)
cvtbl (r6),-(sp)
movl (r10),r5
calls $12/4,*12(r5)
calls $0/4,_spl0
ret
.globl _raioctl
.align 1
_raioctl:.word 0xe00
movw 4(ap),4(ap)
movzwl 4(ap),r5
ashl $-3,r5,r5
bicl2 $0xfffffff8,r5
mull2 $44,r5
addl3 $_radisk,r5,r10
cvtbl 6(r10),r5
bicl2 $0xfffffffa,r5
cmpl r5,$5; jeql L44
movb $5,_u+197
ret
L44:movl 12(ap),r11
movl 8(ap),r0
cmpl r0,$29957; jgtr L80
subl3 $29952,r0,r1
jlss L80
movl 2f[r1],r0
jmp (r0)
2:.long L51
.long L63
.long L48
.long L75
.long L78
.long L79
L80:L46:movb $25,_u+197
ret
L48:pushl $20
pushl 12(ap)
pushal 24(r10)
calls $12/4,_copyout
tstl r0; jeql L49
movb $14,_u+197
L49:ret
L51:movl 4(r11),r5
jlss L54
cmpl r5,28(r10); jleq L52
L54:movb $5,_u+197
ret
L52:clrl r9
jbr L58
L55:movl $512,_u+292
addl3 24(r10),4(r11),r5
ashl $9,r5,-(sp)
moval _u+296,r1
calls $4/4,_ltoL
mull3 28(r10),r9,r5
ashl $9,r5,-(sp)
movq _u+296,-(sp)
moval _u+296,r1
calls $12/4,_Lladd
movl (r11),_u+288
clrb _u+196
clrb _u+197
pushal _minphys
pushl $1
movzwl 4(ap),-(sp)
pushal _rctbuf
pushal _rastrategy
calls $20/4,_physio
tstb _u+197; jneq L61
jbr L57
L61:L56:incl r9
L58:cvtbl 43(r10),r5
cmpl r9,r5; jlss L55
L57:ret
L63:bicl3 $0xfffffffd,16(ap),r5
jneq L64
movb $9,_u+197
ret
L64:movl 4(r11),r5
jlss L68
cmpl r5,28(r10); jleq L66
L68:movb $5,_u+197
ret
L66:clrl r9
jbr L72
L69:movl $512,_u+292
addl3 24(r10),4(r11),r5
ashl $9,r5,-(sp)
moval _u+296,r1
calls $4/4,_ltoL
mull3 28(r10),r9,r5
ashl $9,r5,-(sp)
movq _u+296,-(sp)
moval _u+296,r1
calls $12/4,_Lladd
movl (r11),_u+288
clrb _u+196
pushal _minphys
clrl -(sp)
movzwl 4(ap),-(sp)
pushal _rctbuf
pushal _rastrategy
calls $20/4,_physio
clrb _u+197
L70:incl r9
L72:cvtbl 43(r10),r5
cmpl r9,r5; jlss L69
L71:ret
L75:bicl3 $0xfffffffd,16(ap),r5
jneq L76
movb $9,_u+197
ret
L76:cvtwl 8(r11),-(sp)
pushl (r11)
pushl 4(r11)
movzwl 4(ap),-(sp)
calls $16/4,_rareplace
ret
L78:bisb2 $64,6(r10)
ret
L79:pushal _radg
pushal _raseql
clrl -(sp)
pushl $1
movzwl 4(ap),r5
ashl $-3,r5,r5
bicl2 $0xfffffff8,r5
moval _raaddr+1[r5],r4
cvtbl (r4),-(sp)
moval _raaddr[r5],r5
cvtbl (r5),-(sp)
movl *(r10),r5
calls $24/4,(r5)
ret
L47:ret
.align 1
_rareplace:.word 0xf80
bicl3 $0xffffff00,4(ap),r5
ashl $-3,r5,r5
bicl3 $0xfffffff8,r5,r8
mull3 r8,$44,r5
addl3 $_radisk,r5,r10
moval 0[r8],r5
addl3 $_raaddr,r5,r9
calls $0/4,_spl6
jbr L82
L81:bisb2 $16,6(r10)
pushl $26
pushal 6(r10)
calls $8/4,_sleep
L82:cvtbl 6(r10),r5
bicl2 $0xfffffff7,r5
tstl r5; jneq L81
L83:bisb2 $8,6(r10)
pushl 12(ap)
pushl 8(ap)
pushl r8
pushal _84
calls $16/4,_printf
cvtbl (r9),-(sp)
movl (r10),r5
calls $4/4,*4(r5)
movl r0,r11
pushl $36
pushl r11
calls $8/4,_bzero
addl3 $1,_rarefno,r5
movl r5,_rarefno
movl r5,(r11)
movw 2(r9),4(r11)
movb $20,8(r11)
movl 12(ap),12(r11)
movl 8(ap),28(r11)
movl 16(ap),r7
jeql L86
movl $1,r7
 # label
L86: # label
L87:movw r7,10(r11)
movl (r11),8(r10)
movb $20,12(r10)
pushl r11
clrl -(sp)
cvtbl (r9),-(sp)
movl (r10),r5
calls $12/4,*12(r5)
jbr L89
L88:pushl $25
pushal 7(r10)
calls $8/4,_sleep
L89:cvtbl 6(r10),r5
bicl2 $0xffffffdf,r5
tstl r5; jeql L88
L90:movb 7(r10),_u+197
cvtbl 6(r10),r5
bicl2 $0xffffffef,r5
tstl r5; jeql L91
pushal 6(r10)
calls $4/4,_wakeup
L91:bicb2 $56,6(r10)
calls $0/4,_spl0
ret
.globl _raseql
.align 1
_raseql:.word 0xfc0
movl 12(ap),r11
tstb 8(r11); jneq L93
cmpw 10(r11),$255; jneq L93
pushl 4(ap)
calls $4/4,_rareset
ret
L93:clrl r8
jbr L98
L95:moval _raaddr[r8],r5
cvtbl (r5),r5
cmpl r5,4(ap); jneq L100
moval _raaddr+1[r8],r5
cvtbl (r5),r5
cmpl r5,8(ap); jneq L100
moval _raaddr+2[r8],r5
cmpw (r5),4(r11); jneq L100
jbr L97
L100:L96:incl r8
L98:cmpl r8,_racnt; jlss L95
L97:cmpl r8,_racnt; jlss L102
cvtbl 8(r11),-(sp)
cvtwl 10(r11),-(sp)
pushl 8(ap)
pushl 4(ap)
cvtwl 4(r11),-(sp)
pushal _104
calls $24/4,_printf
ret
L102:mull3 r8,$44,r5
addl3 $_radisk,r5,r9
cvtwl 10(r11),r5
bicl3 $0xffffffe0,r5,r6
cmpl r6,$4; jeql L107
cmpl r6,$3; jneq L105
L107:bicb2 $1,6(r9)
L105:L108:cvtbl 8(r11),r5
bicl3 $0xffffff00,r5,r0
cmpl r0,$148; jeql L147
jgtr L152
cmpl r0,$137; jgtr L109
subl3 $128,r0,r1
jlss L109
movl 2f[r1],r0
jmp (r0)
2:.long L111
.long L109
.long L109
.long L139
.long L143
.long L109
.long L109
.long L109
.long L138
.long L137
L153:L152:cmpl r0,$161; jeql L114
cmpl r0,$162; jeql L114
jbr L109
L111:cmpl (r11),8(r9); jneq L112
bisb3 $128,12(r9),8(r11)
jbr L108
L112:L114:movl 16(r9),r10
clrl r7
jbr L118
L115:cmpl (r11),36(r10); jneq L119
jbr L117
L119:L116:movl r10,r7
movl 12(r10),r10
L118:tstl r10; jnequ L115
L117:tstl r10; jnequ L121
cvtbl 8(r11),r5
bicl3 $0xffffff00,r5,-(sp)
cvtwl 10(r11),-(sp)
pushl (r11)
pushl r8
pushal _123
calls $20/4,_printf
ret
L121:cmpl *16(r10),(r11); jeql L124
movzwl 26(r10),-(sp)
pushl (r10)
pushl (r11)
pushl *16(r10)
pushl r8
pushal _126
calls $24/4,_printf
L124:tstl r7; jeqlu L127
movl 12(r10),12(r7)
jbr L128
L127:movl 12(r10),16(r9)
L128:cmpl r10,20(r9); jnequ L129
movl r7,20(r9)
L129:subl3 12(r11),20(r10),36(r10)
tstl r6; jeql L131
bisl2 $4,(r10)
cvtwl 10(r11),r5
cmpl r5,$8; jeql L135
cmpl r5,$72; jneq L133
L135:movw $6,24(r10)
jbr L134
L133:cvtwl 10(r11),-(sp)
pushl 32(r10)
pushl r8
pushal _136
calls $16/4,_printf
L134:L131:pushl 16(r10)
pushl 4(ap)
movl (r9),r5
calls $8/4,*16(r5)
pushl r10
calls $4/4,_iodone
ret
L137:pushl r11
pushl r9
calls $8/4,_rasonl
ret
L138:bicb2 $1,6(r9)
ret
L139:tstl r6; jeql L140
cvtwl 10(r11),-(sp)
pushl r8
pushal _142
calls $12/4,_printf
ret
L140:movl 28(r11),32(r9)
movw 36(r11),36(r9)
movw 38(r11),38(r9)
movw 40(r11),40(r9)
cvtwl 44(r11),28(r9)
movb 46(r11),42(r9)
movb 47(r11),43(r9)
bisb2 $4,6(r9)
ret
L143:tstl r6; jeql L144
pushl 8(ap)
pushl 4(ap)
pushal _146
calls $12/4,_printf
L144:ret
L147:clrb 7(r9)
tstl r6; jeql L148
cvtwl 10(r11),-(sp)
pushl r8
pushal _150
calls $12/4,_printf
movb $5,7(r9)
L148:bisb2 $32,6(r9)
pushal 7(r9)
calls $4/4,_wakeup
ret
L109:cvtwl 10(r11),-(sp)
cvtbl 8(r11),r5
bicl3 $0xffffff00,r5,-(sp)
pushl 8(ap)
pushl 4(ap)
cvtwl 4(r11),-(sp)
pushal _151
calls $24/4,_printf
ret
L110:ret
.globl _rareset
.align 1
_rareset:.word 0xf00
clrl r11
jbr L157
L154:moval _raaddr[r11],r5
cvtbl (r5),r5
cmpl r5,4(ap); jeql L159
jbr L155
L159:mull3 r11,$44,r5
addl3 $_radisk,r5,r10
movl 16(r10),r9
jbr L164
L161:movl 12(r9),r8
pushl 16(r9)
pushl 4(ap)
movl (r10),r5
calls $8/4,*16(r5)
bisl2 $4,(r9)
pushl r9
calls $4/4,_iodone
L162:movl r8,r9
L164:tstl r9; jnequ L161
L163:clrl 20(r10)
clrl 16(r10)
bicb2 $3,6(r10)
pushl r10
calls $4/4,_wakeup
L155:incl r11
L157:cmpl r11,_racnt; jlss L154
L156:ret
.data
.align 2; _raevents:.long _165
.long _166
.long _167
.long _168
.long _169
.long _170
.long _171
.long _172
.long _173
.long _174
.long _175
.long _176
.text
.globl _radg
.align 1
_radg:.word 0xf80
movl 12(ap),r11
cmpb 8(r11),$2; jneq L177
cmpw 10(r11),$8; jneq L177
tstb 9(r11); jneq L177
ret
L177:cvtbl 9(r11),r5
bicl3 $0xffffff00,r5,-(sp)
cvtwl 10(r11),-(sp)
cvtbl 8(r11),-(sp)
cvtbl 9(r11),r5
bicl2 $0xffffff3f,r5
tstl r5; jeql L183
moval _180,r7
jbr L184
L183:moval _181,r7
 # label
L184:pushl r7
cvtwl 6(r11),-(sp)
pushl 8(ap)
pushl 4(ap)
cvtwl 4(r11),-(sp)
pushal _179
calls $36/4,_printf
cvtwl 10(r11),r5
bicl2 $0xffffffe0,r5
cmpl r5,$11; jgtr L185
cvtwl 10(r11),r5
bicl2 $0xffffffe0,r5
pushl _raevents[r5]
pushal _187
calls $8/4,_printf
L185:cvtbl 8(r11),r0
cmpl r0,$4; jgtr L223
subl3 $0,r0,r1
jlss L188
movl 2f[r1],r0
jmp (r0)
2:.long L190
.long L192
.long L194
.long L201
.long L211
L223:cmpl r0,$64; jeql L213
jbr L188
L190:pushal _191
calls $4/4,_printf
jbr L189
L192:pushl 24(r11)
pushal _193
calls $8/4,_printf
jbr L189
L194:cvtbl 35(r11),-(sp)
cvtbl 34(r11),-(sp)
bicl3 $0xf0000000,40(r11),-(sp)
bicl3 $0xfffffff,40(r11),r5
jnequ L199
moval _196,r7
jbr L200
L199:moval _197,r7
 # label
L200:pushl r7
pushal _195
calls $20/4,_printf
jbr L189
L201:bicl3 $0xf0000000,40(r11),-(sp)
bicl3 $0xfffffff,40(r11),r5
jnequ L204
moval _196,r7
jbr L205
L204:moval _197,r7
 # label
L205:pushl r7
pushal _202
calls $12/4,_printf
addl3 $56,r11,r10
jbr L207
L206:addl3 $-2,r10,r5
movl r5,r10
movzwl (r5),-(sp)
pushal _209
calls $8/4,_printf
L207:addl3 $44,r11,r5
cmpl r10,r5; jgtru L206
L208:pushal _210
calls $4/4,_printf
jbr L189
L211:pushl 48(r11)
pushal _212
calls $8/4,_printf
jbr L189
L213:pushal _214
calls $4/4,_printf
addl3 $44,r11,r8
clrl r9
L215:movzbl (r8)+,-(sp)
pushal _209
calls $8/4,_printf
L216:incl r9
cmpl r9,$10; jlss L215
L217:movzbl 1(r8),-(sp)
movzbl (r8),-(sp)
pushal _221
calls $12/4,_printf
jbr L189
L188:pushal _222
calls $4/4,_printf
L189:ret
.align 1
_raonline:.word 0xe00
subl2 $4,sp
movl 4(ap),r11
movl 8(ap),r10
calls $0/4,_spl6
movl r0,-4(fp)
cvtbl 6(r11),r5
bicl2 $0xfffffffd,r5
tstl r5; jneq L227
bicb2 $4,6(r11)
cvtbl (r10),-(sp)
movl (r11),r5
calls $4/4,*4(r5)
movl r0,r9
pushl $36
pushl r9
calls $8/4,_bzero
addl3 $1,_rarefno,r5
movl r5,_rarefno
movl r5,(r9)
movw 2(r10),4(r9)
movb $9,8(r9)
movl (r9),8(r11)
movb $9,12(r11)
pushl r9
clrl -(sp)
cvtbl (r10),-(sp)
movl (r11),r5
calls $12/4,*12(r5)
bisb2 $2,6(r11)
jbr L227
L226:pushl $60
pushl $24
pushl r11
calls $12/4,_tsleep
L227:cvtbl 6(r11),r5
bicl2 $0xfffffffd,r5
tstl r5; jneq L226
L228:cvtbl 6(r11),r5
bicl2 $0xfffffffe,r5
tstl r5; jneq L229
clrl r0
ret
L229:cvtbl 6(r11),r5
bicl2 $0xfffffffb,r5
tstl r5; jneq L231
cvtbl (r10),-(sp)
movl (r11),r5
calls $4/4,*4(r5)
movl r0,r9
addl3 $1,_rarefno,r5
movl r5,_rarefno
movl r5,(r9)
movw 2(r10),4(r9)
movb $3,8(r9)
clrw 10(r9)
clrw 14(r9)
clrl 28(r9)
pushl r9
clrl -(sp)
cvtbl (r10),-(sp)
movl (r11),r5
calls $12/4,*12(r5)
L231:pushl -4(fp)
calls $4/4,_splx
movl $1,r0
ret
.align 1
_rasonl:.word 0xc00
movl 4(ap),r11
movl 8(ap),r10
cvtbl 6(r11),r5
bicl2 $0xfffffffd,r5
tstl r5; jeql L233
bicb2 $2,6(r11)
pushl r11
calls $4/4,_wakeup
L233:cvtwl 10(r10),r5
bicl2 $0xffffffe0,r5
tstl r5; jeql L235
ret
L235:bisb2 $1,6(r11)
movl 24(r11),r5
cmpl r5,$2147483647; jeql L237
cmpl r5,36(r10); jeql L237
pushl 36(r10)
pushl 24(r11)
subl3 $_radisk,r11,r5
divl3 $44,r5,-(sp)
pushal _239
calls $16/4,_printf
L237:movl 36(r10),24(r11)
ret
.align 1
_racinit:.word 0xc00
cvtbl *8(ap),-(sp)
movl *4(ap),r5
calls $4/4,*4(r5)
movl r0,r11
addl3 $1,_rarefno,r5
movl r5,_rarefno
movl r5,(r11)
movl 8(ap),r5
movw 2(r5),4(r11)
movb $4,8(r11)
clrw 10(r11)
movw $80,14(r11)
clrw 12(r11)
clrw 16(r11)
clrl 24(r11)
clrl 20(r11)
calls $0/4,_spl6
movl r0,r10
pushl r11
clrl -(sp)
cvtbl *8(ap),-(sp)
movl *4(ap),r5
calls $12/4,*12(r5)
pushl r10
calls $4/4,_splx
ret
.text 1
_239:.ascii "ra%d: changed size %d to %d\012\000"
_222:.byte 10,0
_221:.ascii " [%x %x]\012\000"
_214:.ascii "scsi:\000"
_212:.ascii "cyl %d\012\000"
_210:.ascii " xx\012\000"
_209:.ascii " %x\000"
_202:.ascii "%sbn %d;\000"
_197:.byte 114,0
_196:.byte 108,0
_195:.ascii "%sbn %d; lev x%x, retry x%x\012\000"
_193:.ascii "host mem access; addr x%x\012\000"
_191:.ascii "oops\012\000"
_187:.ascii "%s; \000"
_181:.ascii "hard\000"
_180:.ascii "soft\000"
_179:.ascii "ra%d ctl%d typ%d seq %d: %s err; fmt x%x ev x%x fl x%x\012\000"
_176:.ascii "drive err\000"
_175:.ascii "cntl err\000"
_174:.ascii "host buf access err\000"
_173:.ascii "data err\000"
_172:.ascii "comp err\000"
_171:.ascii "write prot\000"
_170:.ascii "med fmt\000"
_169:.ascii "available\000"
_168:.ascii "offline\000"
_167:.ascii "op aborted\000"
_166:.ascii "inv cmd\000"
_165:.ascii "ok\000"
_151:.ascii "ra%d ctl%d typ%d: stray mscp msg opcd 0%o sts x%x\012\000"
_150:.ascii "ra%d: rpl sts x%x\012\000"
_146:.ascii "ra ctl%d typ%d: bad init\012\000"
_142:.ascii "ra%d: can't get unit sts x%x\012\000"
_136:.ascii "err on ra%d block %D: sts x%x\012\000"
_126:.ascii "ra%d sent %d got %d crf; flg %x dev %x\012\000"
_123:.ascii "ra%d stray end: crf %d sts x%x opcode 0%o\012\000"
_104:.ascii "ra%d ctl%d typ%d: stray mscp packet sts x%x opcode %o\012\000"
_84:.ascii "ra%d replace %D with %D\012\000"
.lcomm _rctbuf,44
.lcomm _rarefno,4
.globl _nswdevt
.comm _nswdevt,4
.globl _zvms
.comm _zvms,40
.globl _bclnlist
.comm _bclnlist,4
.globl _bswlist
.comm _bswlist,44
.globl _bfreelist
.comm _bfreelist,132
.globl _swpf
.comm _swpf,4
.globl _swsize
.comm _swsize,4
.globl _nbuf
.comm _nbuf,4
.globl _buffers
.comm _buffers,4
.globl _buf
.comm _buf,4
