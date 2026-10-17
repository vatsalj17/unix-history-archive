.nr DO \n(.o
.nr tm 0
.de RT\"	RT - reset everything to normal state
.if !\\n(1T .BG
.if !\\n(IK .di
.ce 0
.ul 0
.if \\n(IP .in -\\n(IQu
.if !\\n(IP .nr IQ 5m
.if \\n(IP .nr IP -1m
.ft R
.ta 6 11 16 21 26 31 36 41 46 61 56 61 66 71 76 81
.ps \\n(PS
.vs \\n(VSp
.fi
..
.de IZ\"	IZ - initialization
.br
.nr y 0-6
.nr x 0 1
.nr IP 0
.nr IQ 5m
.nr PS 10
.nr VS 12
.ps \\n(PS
.vs \\n(VSp
.nr TB 0
.nr LL \n(.lu
.ll \\n(LLu
.nr LT \\n(.l
.lt \\n(LTu
.tc ~
.tr ~
.ds CH "- % -
.ds Ch \\*(CH
.ds CF "\\*(DY
.wh 0 NP
.wh -6 FO
.wh -3 BT
.nr EP \n(.pu-6v
..
.de TM
.pi /usr/bin/col
.nr ST 1
.ds MN \\$1
.nr MM \\n(.$
.nr tm 1
.if \\n(.$-1 .ds CA \\$2
.if \\n(.$-2 .ds CC \\$3
.rm RP
.rm S0
.rm S2
.rm AX
..
.de RP
.nr ST 2
.rm SG
.rm CS
.rm TM
..
.de TL\"	TL - title and initialization
.nr TV 1
.di WT
.na
.fi
.ll 50
.if \\n(tm .ll 30
.ft B
.if !\\n(tm .ps 12
..
.de TX
.rs
.sp 3
.ce 1000
.ul 1000
.ps 12
.ft B
.vs 15p
.ne 4
.WT
..
.de AU\"	AU - author(s)
.nr AV 1
.ad
.br
.di
.br
.nf
.nr NA +1
.ds R\\n(NA \\$1
.ds E\\n(NA \\$2
.di A\\n(NA
.ll \\n(LLu
.if !\\n(tm .ft I
.if \\n(tm .ft B
.if \\n(tm .ll 16
.ps 10
..
.de AX
.ft R
.rs
.ce 1000
.ul 0
.ps 10
.vs 12p
.sp 2
.A1
.sp
.I1
.if \\n(NA-1 .sp 2
.A2
.if \\n(NA-1 .sp
.I2
.if \\n(NA-2 .sp 2
.A3
.if \\n(NA-2 .sp
.I3
.if \\n(NA-3 .sp 2
.A4
.if \\n(NA-3 .sp
.I4
.if \\n(NA-4 .sp 2
.A5
.if \\n(NA-4 .sp
.I5
..
.de AI\"	AI - authors institution
.br
.ft R
.di
.di I\\n(NA
.nf
..
.de AB\"	AB - begin an abstract
.br
.di
.ll \\n(LTu
.nr 1T 1
.nr IK 1
.di WB
.rs
.ce 1
.ul
.ll \\n(LTu
ABSTRACT
.sp
.fi
.nr oi \\n(.i
.in +(\\n(.lu/12u)
.ll -(\\n(.lu/12u)
.br
.ps \\n(PS
.ti +5
..
.de AE\"	AE - end of an abstract
.br
.di
.ll \\n(LLu
.nr 1T 0
.nr IK 0
.in \\n(oiu
.di
.ce 0
.if \\n(ST-1 .SY
.rm SY
..
.de OK\"	OK - "other keywords" for cover sheet.
.br
.di
.di OD
.nf
.ft B
..
.de CS\"	CS - spew out cover sheet
.br
.di
.tr ~
.sp 6
.nf
      \s24\(bs\s12\fB  Bell Laboratories        \s16\fRCover Sheet for Technical Memorandum
.sp
.fi
.ps 9
.ft I
The information contained herein is for the use of employees of Bell Laboratories and is not for publication. (See GEI 13.9-3)\p
.ft R
.nf
.sp
.ll 30
.ps 9
.ta 10 40 47
.mk
Title- \fB\s10
.rt
.in 10
.WT
.rt
.ll \\n(LLu
.in 40
\s9\fRDate-\s10\fB
.rt
.in 47
\\*(DY
.sp
.ti 40
.mk
\s9\fRTM-\fB\s10
.br
.rt
.CT \\*(MN
.rt
.in 0
.sp
.mk
\s9\fROther Keywords-\fB
.rt
.in 20
.OD
.rm OK
.rm OD
.in 0
.sp 3
.ps 9
.ta 20 35 45
.ft R
Author	Location	Extension	Charging Case- \s10\fB\\*(CA
.ps 10
.nf
.mk
.A1
.rt
	\\*(R1	\\*(E1	\fR\s9Filing Case- \s10\fB\\*(CC
.if \\n(NA-1 .CB 2
.if \\n(NA-2 .CB 3
.if \\n(NA-3 .CB 4
.if \\n(NA-4 .CB 5
.ft R
.sp 2
.WB
.br
.sp 5
.FF
.rm FF
.nr x 50-\\n(nl
.sp \\nx
.nf
.ta 15 30 45
Pages Text  \\$1	Other   \\$2	Total   \\$3
.sp
No. Figures  \\$4	No. Tables   \\$5	No. Refs.   \\$6
.ps 7
.sp
.tl 'E-1932-C (6-73)'SEE REVERSE SIDE FOR DISTRIBUTION LIST''
.ll \\n(LLu
.fi
.ps 10
.bp 1
..
.de CB\"	CB - help with cover sheet
.br
.mk
.A\\$1
.rt
.if \\$1-1 .if !\\$1-2 \&	\\*(R2	\\*(E2
.if \\$1-2 .if !\\$1-3 \&	\\*(R3	\\*(E3
.if \\$1-3 .if !\\$1-4 \&	\\*(R4	\\*(E4
.if \\$1-4 .if !\\$1-5 \&	\\*(R5	\\*(E5
.br
..
.de CT
\\$1
\\$2
\\$3
\\$4
..
.de S1\"	S1 - tm style
.sp 6
.ta 47
.ll 80
.ps 36
.br
.vs .25i
.ps 12
.br
.ft B
.nf
\tBell Laboratories
.ft R
.fi
.na
.ps 8
.vs 12p
.sp 3
.mk
.fi
.ll
.ll 40
Subject:
.ft B
.ps
.WT
.ft R
.br
.if \\n(MM-1 \\s8Case- \\*(CA\\s10
.if \\n(MM-2 --\s8 File- \\*(CC\\s10
.br
.rt
.ll 70
.in 52
.ps 8
.mk
.ti -6n
date:
.ps \\n(PS
.ft B
.br
.rt
\\*(DY
.sp
.ft R
.ps 8
.mk
.ti -6n
from:
.ps
.br
.rt
.ft B
.A1
.br
.A2
.br
.A3
.br
.A4
.br
.A5
.ft R
.sp
.mk
.ps 8
.ti -6n
.ft R
.if \\n(MM TM:
.br
.rt
.ps
.ft B
.hy 0
.ll 10n
.if \\n(MM \\*(MN
.hy
.ll
.ft R
.ll \\n(LLu
.in
.sp
.ad
.ce
.ft I
MEMORANDUM FOR FILE
.ft R
.sp
.ad
..
.de SY\"	SY - cover sheet of released paper
.ll \\n(LLu
.sp 6
.ns
.if \\n(TV .TX
.if \\n(AV .AX
.ce 0
.nf
.sp 3
.WB
.fi
..
.de S2\"	S2 - first text page, released paper format
.SY
.rm SY
.bp 1
.if \\n(TV .TX
.if \\n(AV .AX
.ce 0
.ft R
.ad
..
.de S0\"	S0- mike lesk conserve paper style
.ll \\n(LLu
.rs
.sp 6
.ns
.if \\n(TV .TX
.if \\n(AV .AX
.ce 0
.nf
.WB
.fi
.ad
..
.de SG\"	SG - signature
.nf
.in +(2u*\\n(.lu/3u)
.sp 2
.A1
.if \\n(NA-1 .sp 2
.A2
.if \\n(NA-2 .sp 2
.A3
.if \\n(NA-3 .sp 2
.A4
.if \\n(NA-4 .sp 2
.A5
.in
\\$1
.br
..
.de Ta\"	Tables. TS - table start, TE - table end
.br
.RT
.ul 0
.sp
.nf
..
.de TE
.fi
.sp 1
..
.de DS\"	DS - display. If .DS C, center; L, left-adjust; I, indent.
.KS
.\\$1D
..
.de CD
.ce 1000
.nf
.nr oi \\n(.i
.sp 1
..
.de D
.ID
..
.de ID
.nf
.nr oi \\n(.i
.sp 1
.in +8
..
.de LD
.nf
.nr oi \\n(.i
.sp 1
..
.de DE\"	DE - display end
.ce 0
.in \\n(oiu
.KE
.sp 1
.fi
..
.de KS\"	KS keep - for keep release features. As in IBM
.if !\\n(IK .KQ
.nr IK +1
..
.de KQ\"	KQ - real keep processor
.br
.di KK
.nr TB 0
..
.de KF\"	KF - floating keep
.if !\\n(IK .FQ
.nr IK +1
..
.de FQ\"	FQ real floating keep processor
.br
.di KK
.nr TB 1
..
.de KE\"	KE release - everything between keep and release is together
.if \\n(IK .if !\\n(IK-1 .RQ
.if \\n(IK .nr IK -1
..
.de RQ\"	RQ real release
.br
.di
.if \\n(dn+\\n(nl-\\n(EP .if !\\n(TB .sp 200
.if !\\n(dn+\\n(nl-\\n(EP .if \\n(TB .nr TB 0
.nf
.in 0
.if !\\n(TB .KK
.in
.fi
..
.de EQ\"	EQ default equation breakout
.sp
.ce
.ne 2
.ds EL \\$1
.if \\n(.$-1 .ds EL \\$2
.nr LE \\n(.$
.@\\$1
..
.de @L\"	This nonsense permits recognition of .EQ L, .EQ I, .EQ C
.ce0\"		by invoking macros @L, @I, and so forth
.nr LE -1
..
.de @C
.nr LE -1
..
.de @I
.ce 0
.ti 10n
.nr LE -1
..
.de EN
.ce 0
.lt \\n(.lu
.if \\n(LE .sp -1
.if \\n(LE .tl '''\\*(EL'
.sp 1
.ns
..
.de EM\"	EM end up macro - process left over keep-release
.if \\n(TB .br
.if \\n(TB 
.if \\n(TB .NP
..
.de NP\"	NP new page
'sp 3
.ft R
.lt \\n(LTu
.ps \\n(PS
.PT
.ps
.ft P
'sp 3
.nr x 0 1
.nr y 0-6
.if \\n(MF .fz
.nr MF 0
.ns
.os
.ev 1
.if \\n(TB .nf
.if \\n(TB .KK
.if \\n(TB .fi
.ev
.nr TB 0
.mk
..
.de PT
.if \\n%-1 .tl '\\*(LH'\\*(Ch'\\*(RH'
..
.de FO\"	FO - footer of page
.nr FC +1
.nr dn 0
.if \\nx .xf
.nr MF 0
.if \\n(dn .nr MF 1
.ch FO -6
.if !\\n(FC-1 .if \\n(NX .RC
.if !\\n(FC-1 .if !\\n(NX 'bp
.nr FC -1
..
.de 2C\"	2C - begin double column
.if !\\n(1T .sp 4
.RT
.mk
.nr NC 1
.nr NX 1
.nr L1 \\n(.l
.nr L2 \\n(.l*7/15
.ll \\n(L2u
.FL \\n(L2*11/12
.nr RO \\n(L2*8/7
.ns
..
.de RC
.if \\n(NC-1 .C2
.if !\\n(NC-1 .C1
.nr NC \\n(ND
..
.de C1
.rt
.po +\\n(ROu
.nr ND 2
.nr x 0 1
.ns
..
.de C2
.po \\n(DOu
'bp
.nr ND 1
..
.de 1C\"	1C - return to single column format
.nr NX 0
.br
.po \\n(DOu
.nr ND 1
.ll \\n(L1u
.bp
..
.de R3
.ll 120
.pl 102
.nr LT \\n(.l
..
.de BT
.ft R
.lt \\n(LTu
.tl '\\*(LF'\\*(CF'\\*(RF'
.ft P
..
.de PP\"	PP - paragraph
.RT
.ne 2
.if \\n(1T .sp
.ti +5
..
.de SH\"	SH - (unnumbered) section heading
.RT
.if \\n(1T .sp 1
.if !\\n(1T .BG
.RT
.ne 7
.ft B
.ul 1000
..
.de NH\"	NH - numbered heading
.RT
.if \\n(1T .sp 1
.if !\\n(1T .BG
.RT
.ne 7
.ft B
.ul 1000
.nr a \\$1
.if !\\n(.$ .nr a 1
.if !\\na .nr a 1
.nr H\\na +1
.if !\\na-4 .nr H5 0
.if !\\na-3 .nr H4 0
.if !\\na-2 .nr H3 0
.if !\\na-1 .nr H2 0
.if !\\$1 .if \\n(.$ .nr H1 1
.ds SN \\n(H1.
.if \\na-1 .as SN \\n(H2.
.if \\na-2 .as SN \\n(H3.
.if \\na-3 .as SN \\n(H4.
.if \\na-4 .as SN \\n(H5.
\\*(SN
..
.de BG\"	BG - begin, execute at first PP
.br
.di
.ce 0
.nr 1T 1
.S\\n(ST
.rm S0
.rm S1
.rm S2
.rm OK
.rm OD
.rm TX
.rm AX
.rm WT
.rm I1
.rm I2
.rm I3
.rm I4
.rm I5
.rm CB
.rm E1
.rm R1
.rm R2
.rm E2
.de TL
.ft B
.sp
.ul 100
.ce 100
.LG
\\..
.de AU
.ft I
.ul 0
.ce 100
.sp
.NL
\\..
.de AI
.ft R
.ce 100
.ul 0
.sp
.NL
\\..
.ds Ch \\*(CH
.sp 2
.fi
.ll \\n(LLu
..
.de IP\"	IP - indented paragraph
.RT
.if !\\n(IP .nr IP +1m
.sp
.ne 3
.if \\n(.$-1 .nr IQ \\$2m
.LB "\\$1"
..
.de LP\"	LP - left aligned (block) paragraph
.RT
.sp
.ne 3
.if \\n(.$ .LB \\$1
..
.de IE\"	IE - synonym for .LP
.LP
..
.de LB\"	LB - label paragraph
.in +\\n(IQu
.ta \\n(IQu+1m
.ti -\\n(IQu
\\$1\t\c
..
.de RS\"	RS - prepare for double indenting
.in +\\n(IQu
.nr IP +1m
..
.de RE\"	RE - retreat to the left
.br
.in -\\n(IQu
..
.de CM\"	CM - cut mark
.po 0
.tl '-'''
.po
..
.de B\"	B - bold font
.ft B
.ul 1000
..
.de R\"	R - Roman font
.ft R
.ul 0
..
.de I\"	I - italic font
.ft I
.ul 1000
..
.de TA\"	TA - tabs set in ens or chars
.ta \\$1n \\$2n \\$3n \\$4n \\$5n \\$6n \\$7n \\$8n \\$9n
..
.de SM\"	SM - make smaller size
.ps -2
..
.de LG\"	LG - make larger size
.ps +2
..
.de NL\"	NL - return to normal size
.ps \\n(PS
..
.de DA\"	DA - force date; ND - no date or new date.
.if \\n(.$ .ds DY \\$1 \\$2 \\$3 \\$4
.ds CF \\*(DY
..
.de ND
.ds DY \\$1 \\$2 \\$3 \\$4
.rm CF
..
.if \n(mo-0 .ds mo January
.if \n(mo-1 .ds mo February
.if \n(mo-2 .ds mo March
.if \n(mo-3 .ds mo April
.if \n(mo-4 .ds mo May
.if \n(mo-5 .ds mo June
.if \n(mo-6 .ds mo July
.if \n(mo-7 .ds mo August
.if \n(mo-8 .ds mo September
.if \n(mo-9 .ds mo October
.if \n(mo-10 .ds mo November
.if \n(mo-11 .ds mo December
.if \n(dw-0 .ds dw Sunday
.if \n(dw-1 .ds dw Monday
.if \n(dw-2 .ds dw Tuesday
.if \n(dw-3 .ds dw Wednesday
.if \n(dw-4 .ds dw Thursday
.if \n(dw-5 .ds dw Friday
.if \n(dw-6 .ds dw Saturday
.ds DY \*(mo \n(dy, 19\n(yr
.IZ
.rm IZ
.rm mo
.de FN
.FS
..
.de FS\"	FS - begin footnote
'ce 0
.di
.da FF
.ev 1
.if !\\n+x-1 .fs
..
.de FE\"	FE - footnote end
.br
.in 0
.ev
.di
.nr y -\\n(dn
.ch FO \\nyu
.if \\n(nl-\\n(.p-\\ny .ch FO \\n(nlu+1
..
.de fs\"	SEPARATOR
__________________________________________________________
.br
..
.de fz
.FS
.nf
.fy
.fi
.FE
..
.de fx
.di fy
..
.de xf
.ev 1
.nf
.FF
.rm FF
.di
.fi
.ev
..
.de FL
.ev 1
.ll \\$1
.ev
..
.ch FO 100
.wh -6 fx
.ch FO -6
.ev 1
.ll \n(LLu
.ll -(\n(.lu/12u)
'ps8
'vs10p
.ev
.br
.em EM
