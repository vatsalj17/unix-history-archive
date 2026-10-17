/*
 * conf_bits.c
 *
 * encode or decode a bitmapped integral value
 */

#include <sys/compat.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Max # of bits conveniently represented in an integral value. */
#define MAX_BITS	(sizeof(long) * CHAR_BIT)

int		num_bits;
int		encode;
char *		cmd;

#if USE_PROTO
int do_encode(int argn, char * args[])
#else
int
do_encode(argn, args)
int argn;
char * args[];
#endif
{
	long		bit_num;
	unsigned long	mask;
	char *		tail;
	int		argm;
	unsigned long	value;

	value = 0L;

	for (argm = 0; argm < argn; argm++) {

		bit_num = strtol(args[argm], & tail, 0);

		if (tail == args[argm]) {
			fprintf(stderr,	"%s: argument %s invalid, must be "
					"integral value\n",
				cmd, args[argm]);
			return 1;
		}

		if (bit_num >= num_bits) {
			fprintf(stderr, "%s: Invalid value %d, must be "
					"less than %d\n",
				cmd, bit_num, num_bits);
			return 1;
		}

		mask = 1UL << bit_num;
		value |= mask;
	}

	printf("%d\n", value);
	return 0;
}


#if USE_PROTO
int do_decode(int argn, char * args[])
#else
int
do_decode(argn, args)
int argn;
char * args[];
#endif
{
	unsigned long	value;
	char *		tail;
	int		bit;
	unsigned long	mask;

	if (argn > 0) {

		value = strtol(args[0], & tail, 0);

		if (tail == args[0]) {
			fprintf(stderr, "%s: argument %s invalid, must be "
					"integral value\n",
				cmd, args[0]);
			return 1;
		}

		if (value >= (1UL << num_bits)) {
			fprintf(stderr, "%s: argument %s invalid, must be "
					"less than %d\n",
				cmd, args[0], 1UL << num_bits);
			return 1;
		}

	} else
		value = 0L;

	for (bit = 0; bit < num_bits; bit++) {

		mask = value & (1UL << bit);

		if (mask)
			printf("%d ", bit);
	}
	putchar('\n');
	return 0;
}


#if USE_PROTO
void usage(void)
#else
void
usage()
#endif
{
	fprintf (stderr, "Usage:\t%s -e,--encode num_bits val1 val2 ...\n"
			  "\t%s -d,--decode num_bits value\n", cmd, cmd);
	exit (1);
}


#if USE_PROTO
int main(int argc, char * argv[])
#else
int
main(argc, argv)
int argc;
char * argv[];
#endif
{
	char *		tail;

	cmd = argv[0];

	if (argc < 2)
		usage();

	if (strcmp(argv[1], "--encode") == 0 || strcmp (argv[1], "-e") == 0)
		encode = 1;
	else if (strcmp(argv[1], "--decode") == 0 ||
		 strcmp (argv[1], "-d") == 0)
		encode = 0;
	else
		usage();

	num_bits = (int)strtol(argv[2], &tail, 0);

	if (tail == argv[2])
		usage();

	if (num_bits < 1 || num_bits > MAX_BITS) {
		fprintf(stderr,
		"%s: Bit width = %d invalid, must be between 1 and %d\n",
		cmd, num_bits, MAX_BITS);
		exit(1);
	}

	return (encode ? do_encode : do_decode) (argc - 3, argv + 3);
}


