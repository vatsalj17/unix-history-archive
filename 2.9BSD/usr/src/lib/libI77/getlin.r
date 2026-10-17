include(rat_io.h)
##
## @(#)getlin.r	1.1
##
## getlin - read next line from a file
##
integer function getlin(line, f) 
character*(*) line
integer f 

	character c, getch
	integer*2 pos
 
	for (pos = 0; getch(c, f) != EOF; ) {
		if (pos < len(line)) {
			pos = pos + 1 
			line(pos:pos) = c 
		} 
		if (c == NEWLINE) 
			break 
	} 
	if (pos == 0 & c == EOF) 
		pos = ichar(EOF) 
	return (pos)

end 
