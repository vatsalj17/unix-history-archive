# Test printn and scann routines
integer*4 ll, al(100)
integer fileid
integer l
integer lenstr
integer open, getlin
integer scann, fscann, sscann
character*100 line
character*20 string
character*100 sprntn
real x, ax(100)
double precision d, ad(100)
integer i, ai(100)
integer*4 lzero
data lzero /0/

	l = sscann('12345, 54321 ', '%d, %ld', i, ll)
	call printn('%d fields:  i = %d, ll = %ld\\n', l, i, ll)
	call putlin('Enter a line:\\n', 2)
	l = getlin(line, 0)
	call printn('%d characters\\n', l)
	call putlin(line(1:l), 1)
	call printn("'%s'\\n", line(1:l))
	call fprntn(1, '"%s"\\n', line(1:l))
	string = sprntn("'%s'", line(1:l))
	l = lenstr(string)
	call fprntn(2, "l = %d\\n", l)
	call putlin(string(1:l), 1)

	call putlin('\\nEnter a number:', 2)
	l = scann('%F', d)
	call printn('You entered %lf', d)

	x = 123.456
	d = 1234.56789012345d0
	i = 32100
	ll = 987654321
	call fprntn(2, '\\n')
	call fprntn(2, 'i = %d, %o, %x\\n', i, i, i)
	call fprntn(2, 'll = %n, %ld, %lo, %lx\\n', ll, ll, ll, ll)
	call fprntn(2, 'll = %n, %D, %O, %X\\n', ll, ll, ll, ll)
	call fprntn(2, 'x = %e, %f, %g\\n', x, x, x)
	call fprntn(2, 'd = %.15e, %.12f, %.15g\\n', d, d, d)
	call fprntn(2, 'd = %.15le, %.12lf, %.15lg\\n', d, d, d)

	fileid = open('testin', 'r')
	l = fscann(fileid, '%100{ %d%}', ai)
	call fprntn(2, '\\n%100a%10(%10(%5d%)\\n%)', ai)
	call seek(fileid, lzero, lzero)
	l = fscann(fileid, '%100{ %n%}', al)
	call fprntn(2, '\\n%100a%10(%10(%5D%)\\n%)', al)
	call seek(fileid, lzero, lzero)
	l = fscann(fileid, '%100{ %fileid%}', ax)
	call fprntn(2, '\\n%100a%20(%5(%15f%)\\n%)', ax)
	call seek(fileid, lzero, lzero)
	l = fscann(fileid, '%100{ %F%}', ad)
	call fprntn(2, '\\n%100a%20(%5(%15lf%)\\n%)', ad)

end
