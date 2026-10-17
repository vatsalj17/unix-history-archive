/*	@(#)abs.c	2.1	SCCS id keyword	*/
abs(arg)
{

	if(arg < 0)
		arg = -arg;
	return(arg);
}
