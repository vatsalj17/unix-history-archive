int	__zero;

/*
**	ptime	put time in string in form "nnnXnnY" where X&Y can be any of 'd', 'h', 'm', 's'
**	=====
*/

char *ptime( time, sp )
  long time;		/** 31 bit time **/
  char *sp;		/** pointer to 8 character buffer where string will be placed (zero terminated) **/
{
	struct { int hiword; unsigned loword; };
	char	s[8];

  if ( time < 0 )
	goto negativ;

  {
	register	trac, sepc, divn;

	sepc = 'm';	trac = 's';
	divn = 60;

	if ( time > 18000 )	/* more than 300 mins. */
	{
		trac = sepc;
		sepc = 'h';
		time =/ 60;

		if ( time > 10080 )	/* more than 1 week */
		{
			trac = sepc;
			sepc = 'd';
			divn = 24;
			time =/ 60;

			if ( time > 23999 )	/* more than 1000 days */
				trac = 0;
		}
	}

	__zero = 1;

	if ( !trac )
	{
		time =/ divn;
		if ( time.hiword )
			goto overflo;
		__ptn( time.loword, s, 6 );
		s[6] = sepc;
	}
	else
	{
		unsigned	t;

		__ptn( (t=time/divn), s, 3 );
		s[3] = __zero ? ' ' : sepc;
		__ptn( (t=time%divn), &s[4], 2 );
		s[6] = __zero ? '0' : trac;
	}
	s[7] = 0;
  }
  {
	register char	*cp1, *cp2;

	cp1 = s;
out:
	for ( cp2=sp ; *cp2++ = *cp1++ ; );
	return( sp );

negativ:
	cp1 = "negativ";
	goto out;
overflo:
	cp1 = "overflo";
	goto out;
  }
}



/*
**	__ptn	put a decimal number into string with leading spaces
**	=====
*/

char *__ptn( t, s, n )
  register unsigned t,n;
  register char *s;
{
  if ( --n )
	s = __ptn( t/10, s, n );
  if ( (t =% 10) || !__zero )
  {
	__zero = 0;
	*s++ = t + '0';
  }
  else
	*s++ = ' ';
  return( s );
}
