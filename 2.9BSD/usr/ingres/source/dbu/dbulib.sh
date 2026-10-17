echo	'creating new dbu library'
chdir	source/dbu
if	$1x = nocompilex	goto nocom
cc	-c -f -O	*.c
: nocom
if	-r dbulib \
		rm dbulib
ar	rv	dbulib \
	main.o \
	index.o \
	create.o \
	modify.o \
	modupdate.o \
	destroy.o \
	userdestroy.o \
	purgetup.o \
	print.o \
	help.o \
	copy.o \
	save.o \
	rupdate.o \
	update.o \
	convert.o \
	secupdate.o \
	rdwrbatch.o \
	resetrel.o \
	get_seq_atts.o \
	dbuproctab.o \
	display.o \
	pr_tree.o \
	readtree.o \
	pr_prot.o \
	rmqm.o \
	get_p_tid.o \

rm	*.o
chmod	644	dbulib
echo	'New dbu library in .../source/dbu/dbulib'
echo	'Move to .../lib/dbulib'
