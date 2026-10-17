chdir	source/equel
echo	compiling equel in .../source/equel
echo	yaccing grammar with ../../bin/yyyacc
echo	There will be two shift/reduce conflicts which can be ignored
../../bin/yyyacc	grammar.y
echo	recompiling scanner
cc	-c -O	*.c
if	-r ../../lib/equel \
		rm 	../../lib/equel
ar vr	../../lib/equel\
	main.o\
	parser.o\
	yylex.o\
	y.tab.o\
	cvar.o\
	include.o\
	name.o\
	getkey.o\
	number.o\
	operator.o\
	prtout.o\
	string.o\
	comment.o\
	retrieve.o\
	display.o\
	getch.o\
	cmap.o\
	yyerror.o\
	nalloc.o
chmod	0644 ../../lib/equel
echo	scanner routines are installed in .../lib/equel
echo	removing \*.o
rm	*.o
echo	compiling equel
cc	-f -O	../../lib/equel	-ly	../../lib/gutil
mv	a.out	equel
chmod	711	equel
echo	'new equel in .../source/equel/equel'
echo	'move it to .../bin/equel and /usr/bin/equel'
