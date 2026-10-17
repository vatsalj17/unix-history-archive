##
## @(#)putlin.r	1.1
##
## putlin - write a string to a file
subroutine putlin(line, f) 
character*(*) line 
integer f 

	integer i
  
	do  i = 1, len(line)
		call putch(line(i:i), f) 
	return 

end 
