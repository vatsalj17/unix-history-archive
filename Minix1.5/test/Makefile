CFLAGS= -F -D_MINIX -D_POSIX_SOURCE

$f:	$f.s
	@cc -o $f $f.s
	@chmem =8192 $f >/dev/null

clean:	
	@rm -f *.s *.bak

# The test11 file must be setuid root, which means
# that you should be root when typing 'make all'.  Furthermore, cem needs
# extra memory when compiling test17.c and test18.c.
all:
	cc $(CFLAGS) -o test0 test0.c;		chmem =8192  test0
	cc $(CFLAGS) -o test1 test1.c;		chmem =8192  test1
	cc $(CFLAGS) -o test2 test2.c;		chmem =8192  test2
	cc $(CFLAGS) -o test3 test3.c;		chmem =8192  test3
	cc $(CFLAGS) -o test4 test4.c;		chmem =8192  test4
	cc $(CFLAGS) -o test5 test5.c;		chmem =8192  test5
	cc $(CFLAGS) -o test6 test6.c;		chmem =8192  test6
	cc $(CFLAGS) -o test7 test7.c;		chmem =8192  test7
	cc $(CFLAGS) -o test8 test8.c;		chmem =8192  test8
	cc $(CFLAGS) -o test9 test9.c;		chmem =8192  test9
	cc $(CFLAGS) -o test10 test10.c;	chmem =8192  test10
	cc $(CFLAGS) -o test11 test11.c;	chmem =8192  test11
	cc $(CFLAGS) -o test12 test12.c;	chmem =8192  test12
	cc $(CFLAGS) -o test13 test13.c;	chmem =8192  test13
	cc $(CFLAGS) -o test14 test14.c;	chmem =20000 test14
	cc $(CFLAGS) -o test15 test15.c;	chmem =8192  test15
	cc $(CFLAGS) -o test16 test16.c;	chmem =8192  test16
	cc $(CFLAGS) -o test17 test17.c;	chmem =8192  test17
	cc $(CFLAGS) -o test18 test18.c;	chmem =8192  test18
	cc $(CFLAGS) -o test19 test19.c;	chmem =8192  test19
	cc $(CFLAGS) -o test20 test20.c;	chmem =65000 test20
	cc $(CFLAGS) -o test21 test21.c;	chmem =8192  test21
	cc $(CFLAGS) -o t10a t10a.c;		chmem =8192  t10a
	cc $(CFLAGS) -o t11a t11a.c;		chmem =8192  t11a
	cc $(CFLAGS) -o t11b t11b.c;		chmem =8192  t11b
	rm -f *.bak *.s

	chown bin test*
	chown root test11
	chmod 4755 test11
	chmod 777 .

