/* The include file for the fault handling program.
 *	Lee Ward	7/1/83
 */

/* Opcode definitions */
#define	CVTDF	0x76
#define	ADDF2	0x40
#define	ADDF3	0x41
#define	SUBF2	0x42
#define	SUBF3	0x43
#define	MULF2	0x44
#define	MULF3	0x45
#define	DIVF2	0x46
#define	DIVF3	0x47
#define	ADDD2	0x60
#define	ADDD3	0x61
#define	SUBD2	0x62
#define	SUBD3	0x63
#define	MULD2	0x64
#define	MULD3	0x65
#define	DIVD2	0x66
#define	DIVD3	0x67

/* Masks for operand specifiers */
#define	MODE_MASK	0xf0
#define	REG_MASK	0xf

/* Addressing modes */
/* Literal modes are missing */
#define	INDEXED		0x4
#define	GENERAL		0x5
#define	REG_DEFRD	0x6
#define	AUTO_DEC	0x7
#define	AUTO_INC	0x8
#define	AUTO_INC_DEF	0x9
#define	BYTE_DISP	0xa
#define	BYTE_DISP_DEF	0xb
#define	WORD_DISP	0xc
#define	WORD_DISP_DEF	0xd
#define	LONG_DISP	0xe
#define	LONG_DISP_DEF	0xf

/* Types */
#define	DOUBLE	8
#define	QUAD	8
#define	FLOAT	4
#define	LONG	4
#define	WORD	2
#define	BYTE	1
#define	GARBAGE	0

/* Registers */
#define	PC	0xf
#define	SP	0xe
#define	FP	0xd
#define	AP	0xc

#define	MAXFLOAT	0xffff7fff
#define	MAXDOUBLL	0xffff7fff
#define	MAXDOUBLR	0xffffffff
#define	MINFLOAT	0x00000000
#define	MINDOUBLL	0x00000000
#define	MINDOUBLR	0x00000000

/* Though these are not all representable types for VAX, they are all the types
 * that we can really work with. This is because VAX uses register pairs.
 */
typedef union {
	char	byte;
	short	word;
	long	llong;
} ANYTYPE;
