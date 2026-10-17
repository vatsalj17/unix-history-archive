: send.tst
:
: "sh send.tst >results"
: "diff send.chk results"
:
send this_file=$0 -
~ -lf
abc
~ -q
<-
~ <-=(-  =
<- <> |0
01234567890123456789012345678901234567890123456789012345678901234567890123456789
012345678901234567890123456789012345678901234567890123456789012345678901234567890
~ -g
<- >< |0
012345678901234567890123456789012345678901234567890123456789012345678901234567890
~ +g
~ A=a AA=b
A AA AAA AAAA
~ A=b A=
~ -k
~ A=c
A AA
~ =:A
A AA
~ AA= =:AA
A AAA
~ -k
~ A=^C1 B=^c2 FF=^ff
ABFFBA
~ C=^ccc D=^d G=^3g
CDG
~ -k
~ 10=0123456789 x=x y=yy z=
1010101010101010
1010101010101010x
xzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxzxz
x	xx	xxx	xxxx	xxxxx	xxxxxx
y	yy	yyy	yyyy	yyyyy	yyyyyy
~101 102 
~ 101 102 
~"103"
~ "1"'0'"3'"' '10'4
~ \\105 A=\^C1 AA\=b :t\-2\0:
~ - :t-8:
A	AA	AAA
~!chdir /;ls dev/null
~!ls dev/null
~!chdir /
~ -t -
~!ls passwd dev/null
~!chdir etc
~!ls passwd dev/null
~.
~!ls passwd dev/null
~.
~!gath
~!echo x$1x
~ -y +t
<-
~$ed -
~$a
~$A AA
~$.
~$1,$p
~$q
~~
~$gath - -
~$<-
~$~.
~$~<-=[-
~$<-
~ -k =yq +0abc
excise
~ -q
~ -
0	1	2	3
~.
~ :t-4s6:
~ -
0	1
~ .
~-
<:t-3s2de:>
0	1
<:t20,40,60,80s106m5e:>
x	x	x	x	x
x	x	x	x	x	x
<:t-cs72de:>
1	8	12	16	20	55	undef
1	8	12	16	20	55xxxxxxxxxxxxxxxxx
~ -g
<:m7t-1s8:>
a	b	c
<:m7t-1s18:>
~.
~ :t-8: -
x	x	x
~ -ir
~~comment
abc
~.
~ +l
xyz
~ -l
~ A=a B=b
~-
ABC
~ C=c
ABC
~ +k
ABC
~ =k
ABC
~ +k
ABC
~ -k
ABC
~.
~ -da -
~ C=c =:A
ABCD
~ -k D=d
ABCD
~ =k
ABCD
~.
ABCD
~ -k
this_file
~ this_file=tf =k
this_file
~ -s
~ -i this_file -r this_file =ir
~ .=~
. 106
..
. -k
. 107
~ =sir -yc
~~x
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
~ -h
a	b	c
~ +hc
~ +abcdefghijklmnopqrstuvwxyz
~ -lqf
 !"#$%&'()*+,-./0123456789:;<=>?
@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
`abcdefghijklmnopqrstuvwxyz{|}~
~ +f
 !"#$%&'()*+,-./0123456789:;<=>?
@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
`abcdefghijklmnopqrstuvwxyz{|}~
~ +q -f
@RUN ascii
 !"#$%&'()*+,-./0123456789:;<=>?
@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
`abcdefghijklmnopqrstuvwxyz{|}~
~ =f
 !"#$%&'()*+,-./0123456789:;<=>?
@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
`abcdefghijklmnopqrstuvwxyz{|}~
~ -i -:prompt
~ =f
xyz
~.
