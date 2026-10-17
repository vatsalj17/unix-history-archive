
/*	mapping of classes to numbers and bits	*/
struct centry
	{
	char	*c_name;	/* class name */
	int	c_word;		/* word of class mask in which it fits */
	unsigned c_mask;	/* the mask bit in the word for this class */
	} classes[CMASKSIZE * 16]
	{
	"5.030A", 1, 01,
	"5.030B", 1, 02,
	"5.030C", 1, 04,
	"6.602B", 1, 010,
	"6.602D", 1, 020,
	"6.6201", 1, 040,
	"6.6202", 1, 0100,
	"6.6203", 1, 0200,
	"6.6204", 1, 0400,
	"6.6205", 1, 01000,
	"6.6206", 1, 02000,
	"6.620PT", 1, 04000,
	"6.622", 1, 010000,
	"6.631PT", 1, 020000,
	"6.641PT", 1, 040000,
	"6.656g", 1, 0100000,
	"6.606", 2, 010,
	};
