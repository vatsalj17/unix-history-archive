include rat.h
# cant - print cant open file message
   subroutine cant(buf)
   integer buf(MAXLINE)

   call putlin(buf, ERROUT)
   call error(" : can't open.")
   return
   end
# ctoi - convert string at in(i) to integer, increment i
   integer function ctoi(in, i)
   character in(ARB)
   integer index
   integer d, i
#   string digits "0123456789"
   integer digits(11)
   data digits(1) /DIG0/
   data digits(2) /DIG1/
   data digits(3) /DIG2/
   data digits(4) /DIG3/
   data digits(5) /DIG4/
   data digits(6) /DIG5/
   data digits(7) /DIG6/
   data digits(8) /DIG7/
   data digits(9) /DIG8/
   data digits(10) /DIG9/
   data digits(11) /EOS/

   while (in(i) == BLANK ! in(i) == TAB)
      i = i + 1
   for (ctoi = 0; in(i) ^= EOS; i = i + 1) {
      d = index(digits, in(i))
      if (d == 0)      # non-digit
         break
      ctoi = 10 * ctoi + d - 1
      }
   return
   end
# equal - compare str1 to str2; return YES if equal, NO if not
   integer function equal(str1, str2)
   character str1(ARB), str2(ARB)
   integer i

   for (i = 1; str1(i) == str2(i); i = i + 1)
      if (str1(i) == EOS) {
         equal = YES
         return
         }
   equal = NO
   return
   end
# error - print fatal error message, then die
   subroutine error(buf)
   integer buf(ARB)

   call remark(buf)
   stop
   end
# fcopy - copy file  in  to file  out
   subroutine fcopy(in, out)
   character buf(MAXLINE)
   integer getlin
   integer in, out

   while (getlin(buf, in) ^= EOF)
      call putlin(buf, out)
   return
   end
# getc - read character from standard input
	integer function getc(c)
	integer c
	integer getch

	getc = getch(c, STDIN)
	return
	end
# getlin - read a line
	integer function getlin(buf, f)
	character buf(MAXLINE)
	integer f, i
	integer getch

	for (i = 1; getch(buf(i), f) ^= NEWLINE; i = i + 1) {
		if (buf(i) == EOF) {
			getlin = EOF
			return
		}
		if (i >= MAXLINE-2) {
			i = i + 1
			buf(i) = NEWLINE
			break
		}
	}

	buf(i+1) = EOS
	getlin = i
	return
	end
# index - find character  c  in string  str
   integer function index(str, c)
   character c, str(ARB)

   for (index = 1; str(index) ^= EOS; index = index + 1)
      if (str(index) == c)
         return
   index = 0
   return
   end

define(abs,iabs)
# itoc - convert integer  int  to char string in  str
   integer function itoc(int, str, size)
   integer iabs, mod
   integer d, i, int, intval, j, k, size
   character str(size)
#   string digits "0123456789"
   integer digits(11)
   data digits(1) /DIG0/
   data digits(2) /DIG1/
   data digits(3) /DIG2/
   data digits(4) /DIG3/
   data digits(5) /DIG4/
   data digits(6) /DIG5/
   data digits(7) /DIG6/
   data digits(8) /DIG7/
   data digits(9) /DIG8/
   data digits(10) /DIG9/
   data digits(11) /EOS/

   intval = iabs(int)
   str(1) = EOS
   i = 1
   repeat {            # generate digits
      i = i + 1
      d = mod(intval, 10)
      str(i) = digits(d+1)
      intval = intval / 10
      } until (intval == 0 ! i >= size)
   if (int < 0 & i < size) {      # then sign
      i = i + 1
      str(i) = MINUS
      }
   itoc = i - 1
   for (j = 1; j < i; j = j + 1) {   # then reverse
      k = str(i)
      str(i) = str(j)
      str(j) = k
      i = i - 1
      }
   return
   end
# length - compute length of string
   integer function length(str)
   integer str(ARB)

   for (length = 0; str(length+1) ^= EOS; length = length + 1)
      ;
   return
   end

define(MAXCHARS,10)

# putc - write character to standard output
	subroutine putc(c)
	integer c

	call putch(c, STDOUT)
	return
	end
# putdec - put decimal integer n in field width >=w
	subroutine putdec(n, w)
	character chars(MAXCHARS)
	integer itoc
	integer i, n, nd, w

	nd = itoc(n, chars, MAXCHARS)
	for ( i = nd+1; i <= w; i = i+1 )
		call putc(BLANK)
   for (i = 1; i <= nd; i = i + 1)
	call putc(chars(i))
   return
   end
# putlin - write a line
	subroutine putlin(buf, f)
	character buf(ARB)
	integer f, i

	for (i = 1; buf(i) ^= EOS; i = i + 1)
		call putch(buf(i), f)

	return
	end

# putstr - write a hollerith string to std output unit
	subroutine putstr(string)
	integer*2 string(ARB)
	integer i, c

	for (i = 1; ; i = i + 1) {
		c = string(i)
		if (c == x'41f0')	# branch & link instruction !
			break
		call putc(c/256)
		call putc(mod(c,256))
	}
	return
	end

# remark - write a hollerith string to error output unit
	subroutine remark(string)
	integer*2 string(ARB)
	integer i, c

	for (i = 1; ; i = i + 1) {
		c = string(i)
		if (c == x'41f0')	# branch & link instruction !
			break
		call putch(c/256, STDOUT)
		call putch(mod(c, 256), STDOUT)
	}
	call putch(NEWLINE, STDOUT)
	return
	end
# scopy - copy string at from(i) to to(j)
   subroutine scopy(from, i, to, j)
   character from(ARB), to(ARB)
   integer i, j, k1, k2

   k2 = j
   for (k1 = i; from(k1) ^= EOS; k1 = k1 + 1) {
      to(k2) = from(k1)
      k2 = k2 + 1
      }
   to(k2) = EOS
   return
   end
# type - determine type of character
   character function type(c)
   character c
   integer index
   integer upalf(27)
   integer lowalf(27)
   integer digits(11)
#   string digits "0123456789"
      data digits(1) /DIG0/
      data digits(2) /DIG1/
      data digits(3) /DIG2/
      data digits(4) /DIG3/
      data digits(5) /DIG4/
      data digits(6) /DIG5/
      data digits(7) /DIG6/
      data digits(8) /DIG7/
      data digits(9) /DIG8/
      data digits(10) /DIG9/
      data digits(11) /EOS/
#   string lowalf "abcdefghijklmnopqrstuvwxyz"
   data lowalf(01)/LETA/
   data lowalf(02)/LETB/
   data lowalf(03)/LETC/
   data lowalf(04)/LETD/
   data lowalf(05)/LETE/
   data lowalf(06)/LETF/
   data lowalf(07)/LETG/
   data lowalf(08)/LETH/
   data lowalf(09)/LETI/
   data lowalf(10)/LETJ/
   data lowalf(11)/LETK/
   data lowalf(12)/LETL/
   data lowalf(13)/LETM/
   data lowalf(14)/LETN/
   data lowalf(15)/LETO/
   data lowalf(16)/LETP/
   data lowalf(17)/LETQ/
   data lowalf(18)/LETR/
   data lowalf(19)/LETS/
   data lowalf(20)/LETT/
   data lowalf(21)/LETU/
   data lowalf(22)/LETV/
   data lowalf(23)/LETW/
   data lowalf(24)/LETX/
   data lowalf(25)/LETY/
   data lowalf(26)/LETZ/
   data lowalf(27)/EOS/
#   string upalf "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
   data upalf(01) /BIGA/
   data upalf(02) /BIGB/
   data upalf(03) /BIGC/
   data upalf(04) /BIGD/
   data upalf(05) /BIGE/
   data upalf(06) /BIGF/
   data upalf(07) /BIGG/
   data upalf(08) /BIGH/
   data upalf(09) /BIGI/
   data upalf(10) /BIGJ/
   data upalf(11) /BIGK/
   data upalf(12) /BIGL/
   data upalf(13) /BIGM/
   data upalf(14) /BIGN/
   data upalf(15) /BIGO/
   data upalf(16) /BIGP/
   data upalf(17) /BIGQ/
   data upalf(18) /BIGR/
   data upalf(19) /BIGS/
   data upalf(20) /BIGT/
   data upalf(21) /BIGU/
   data upalf(22) /BIGV/
   data upalf(23) /BIGW/
   data upalf(24) /BIGX/
   data upalf(25) /BIGY/
   data upalf(26) /BIGZ/
   data upalf(27) /EOS/

   if (index(lowalf, c) > 0)
      type = LETTER
   else if (index(upalf, c) > 0)
      type = LETTER
   else if (index(digits, c) > 0)
      type = DIGIT
   else
      type = c
   return
   end
