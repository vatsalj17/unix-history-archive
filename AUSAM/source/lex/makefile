lex: main.o y.tab.o sub1.o sub2.o header.o
	cc -i main.o y.tab.o sub1.o sub2.o header.o -ly -lS -o lex
	echo done
smallex:
	cc -DSMALL main.c y.tab.c sub1.c sub2.c header.c -ly -lS -o smallex
	echo done
y.tab.c: parser.y
	yacc parser.y
main.o: main.c ldefs.c once.c
	cc -c -O main.c
sub1.o: sub1.c ldefs.c
	cc -c -O sub1.c
sub2.o: sub2.c ldefs.c
	cc -c -O sub2.c
header.o: header.c ldefs.c
	cc -c -O header.c
