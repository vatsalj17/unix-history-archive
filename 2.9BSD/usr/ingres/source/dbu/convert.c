# include	"../symbol.h"

convert(inp, outp, sf, slen, df, dlen)

char	*inp;	/* pointer to input */
char	*outp;	/* pointer to the output area */
int	sf;	/* format of the source number */
int	slen;	/* length of the source number */
int	df;	/* format of the dest */
int	dlen;	/* length of the dest */

/*
**	convert  converts  numeric values of one type and length
**	to a different type and length.
**
**	The source numeric can be i1,i2,i4,f4,or f8.
**	The source number will be converted to the
**	type and length specified in the destination.
**	It also  must be one of i1,i2,i4,f4, or f8.
**
**	convert returns 0 is no overflow occured,
**	  else it returns -1
*/

{
	char		number[8];	/* dummy buffer */
	register char	*num;
	register int	sl;
	register int	dl;
	struct
	{
		int	integer;
	};
	struct
	{
		long	longint;
	};
	struct
	{
		float	flt;
	};
	struct
	{
		double	dbl;
	};

	dl = dlen;
	sl = slen;
	num = number;
	bmove(inp, num,  sl);	/* copy number into buffer */

	if (sf != df)
	{
		/* if the source and destination formats are
		   different then the source must be converted
		   to i4 if the dest is int, otherwise to f8 */

		if (df == FLOAT)
		{
			switch (sl)
			{

			  case 1:
				num->dbl = num[0];	/* i1 to f8 */
				break;

			  case 2:
				num->dbl = num->integer;	/* i2 to f8 */
				break;

			  case 4:
				num->dbl = num->longint;	/* i4 to f8 */
			}
		sl = 8;
		}
		else
		{
			/* check if float >  2**31 */
			if (sl == 8)
				num->flt = num->dbl;	/* convert f8 to f4 */

			if (num->flt > 2147483647.0 | num->flt < -2147483648.0)
				return (-1);
			num->longint = num->flt;
			sl = 4;
		}
	}

	/* source is now the same type as destination */
	/* convert lengths to match */

	if (sl != dl)
	{
		/* lengths don't match. convert. */
		if (df == FLOAT)
		{
			if (dl == 8)
				num->dbl = num->flt;	/* f4 to f8 */
			else
				num->flt = num->dbl;	/* f8 to f4 with rounding */
		}
		else
		{
			switch (dl)
			{

			  case 1:
				if (sl == 2)
				{
					if (num->integer > 127 | num->integer < -128)
						return (-1);
					*num = num->integer;	/* i2 to i1 */
				}
				else
				{
					if (num->longint > 127 | num->longint < -128)
						return (-1);
					*num = num->longint;	/* i4 to i1 */
				}
				break;

			  case 2:
				if (sl == 1)
				{
					num->integer = *num;	/* i1 to i2 */
				}
				else
				{
					if (num->longint > 32767 | num->longint <-32768)
						return (-1);
					num->integer = num->longint;	/* i4 to i2 */
				}
				break;

			  case 4:
				if (sl == 1)
					num->longint = *num;	/* i1 to i4 */
				else
					num->longint = num->integer;	/* i2 to i4 */
			}
		}
	}

	/* conversion is complete */
	/* copy the result into outp */

	bmove(num, outp, dl);
	return (0);
}
