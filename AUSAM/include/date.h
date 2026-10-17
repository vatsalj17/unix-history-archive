/*
 *	This is the shape of the struct returned by localtime(ticks)
 */

struct	date
	{
	int		d_sec;
	int		d_min;
	int		d_hour;
	int		d_dyofmon;
	int		d_month;
	int		d_year;
	int		d_dyofwk;
	int		d_dyofyr;
	};
