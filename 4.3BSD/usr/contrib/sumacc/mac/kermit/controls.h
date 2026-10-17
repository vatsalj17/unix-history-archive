/* File controls.h
*  Constants for use with mackermit communications controls.  Masks
*  allow the changing of one specific communications paramater.
*/




#define NUMCNTLS 24

struct contrec
{
	int andpart, orpart;
} masks[NUMCNTLS-5]={
				/* Baud */
	{0xfd00,0x017c},	/* 300 */
	{0xfd00,0x0bd},		/* 600 */
	{0xfd00,0x05e},		/* 1200 */
	{0xfd00,0x3e},		/* 1800 */
	{0xfd00,0x2e},		/* 2400 */
	{0xfd00,0x1e},		/* 3600 */
	{0xfd00,0x16},		/* 4800 */
	{0xfd00,0xe},		/* 7200 */
	{0xfd00,0xa},		/* 9600 */
	{0xfd00,0x4},		/* 19200 */
	{0xfd00,0x0},		/* 57600 */
				/* Bits sent */
	{0xffff,0x0c00},	/* eight bits */
	{0xf3ff,0x0400},	/* seven bits */
				/* parity */
	{0xffff,0x3000},	/* even */
	{0xcfff,0x1000},	/* odd */
	{0xcfff,0x0000},	/* none */
				/* stop bits */
	{0x3fff,0x4000},	/* one */
	{0xffff,0xc000}		/* two */
};

