#define	_DDI_DKI	1
#define	_SYSV4		1

/*
 * This program performs some simple processing on a device master-file,
 * mdevice (4) as specified in the "System File and Devices Reference
 * Manual" for Unix System V, Release 4.
 *
 * The first part of the program inhales the 'mdevice' file. Other processing
 * is dependent on the internal details of the character-device interface
 * used by Coherent - our aim is to supply details for a mapping layer that
 * converts the Coherent device system calling format.
 */
/*
 *-IMPORTS:
 *	<sys/compat.h>
 *		CONST
 *		PROTO
 *		ARGS ()
 *		LOCAL
 *	<stdlib.h>
 *		EXIT_SUCCESS
 *		EXIT_FAILURE
 *		NULL
 *		free ()
 *		malloc ()
 *	<unistd.h>
 *		STDERR_FILENO
 *	"mdev.h"
 *		read_mdev_file ()
 *	"sdev.h"
 *		read_sdev_file ()
 *	"symbol.h"
 *		symbol_init ()
 *	"assign.h"
 *              assign_imajors ()
 *	"mkconf.c"
 *		write_conf_c ()
 */

#include <sys/compat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "mdev.h"
#include "sdev.h"
#include "mtune.h"
#include "stune.h"
#include "symbol.h"
#include "assign.h"
#include "mkconf.h"
#include "ehand.h"
#include "ecodes.h"

/*
 * Some ancient systems do not define EXIT_SUCCESS or EXIT_FAILURE.
 */

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS	0
#define	EXIT_FAILURE	1
#endif


LOCAL int		_report_mode = 0;

/*
 * Test whether we are in report mode or not.
 */

#if	USE_PROTO
int (report_mode) (void)
#else
int
report_mode ARGS (())
#endif
{
	return _report_mode;
}


/*
 * For some applications, we need to be able to report conflicts in a special
 * way. By convention, "item1" is something new and "item2" is the old item
 * that it conflicts with.
 */

#if	USE_PROTO
void (report_conflict) (CONST unsigned char * item1,
			CONST unsigned char * item2, CONST char * msg)
#else
void
report_conflict ARGS ((item1, item2, msg))
CONST unsigned char * item1;
CONST unsigned char * item2;
CONST char    *	msg;
#endif
{
	if (_report_mode)
		fprintf (stdout, "%s ", item2);
	else
		fprintf (stderr, "%s: %s with %s\n", item1, msg, item2);
}

#if	TRACE
/*
 * If some modules have been compiled with debugging on, this enables that
 * debugging output with -V
 */

int	do_debug;
#endif


#if	USE_PROTO
void usage (CONST char * name)
#else
void
usage (name)
CONST char    *	name;
#endif
{
	static char	usage_msg [] = {
		": Low-level device configuration tool\n"
		"Usage:\t-d\t\tGenerate device configuration tables and makefiles\n"
		"\t-t\t\tGenerate tunable parameter tables\n"
		"\t-M mdevice\tSet a line in \"mdevice\" to the values in the argument\n"
		"\t-S sdevice\tSet a line in \"sdevice\" to the values in the argument\n"
		"\t-m mtune\tSet a line in \"mtune\" to the values in the argument\n"
		"\t-s stune\tSet a line in \"stune\" to the values in the argument\n"
		"\t-W\t\tWrite out changed files\n"
		"\t-r\t\tReport conflicts caused by proposed changes\n"
		"\t-I dir\t\tDirectory where files are located\n"
		"\t-X mdev_name\t'mdevice' file path\n"
		"\t-Y sdev_name\t'sdevice' file path\n"
		"\t-x mtune_name\t'mtune' file path\n"
		"\t-t stune_name\t'stune' file path\n"
	};

#ifdef	STDERR_FILENO
	write (STDERR_FILENO, name, strlen (name));
	write (STDERR_FILENO, usage_msg, sizeof (usage_msg) - 1);
#else
	(void) fprintf (stderr, "%s%s", name, usage_msg);
#endif
}


/*
 * Create a copy of a filename with a ".new" suffix appended.
 */

#if	USE_PROTO
LOCAL CONST char * make_new (CONST char * name, CONST char * prev)
#else
LOCAL CONST char *
make_new (name, prev)
CONST char    *	name;
CONST char    *	prev;
#endif
{
	CONST char    *	temp;

	if (prev != NULL)
		return prev;

	if ((temp = malloc (strlen (name) + 5)) == NULL)
		throw_error (NO_MEMORY,
			     "cannot allocate pathname memory for %s.new",
			     name);
	sprintf (temp, "%s.new", name);
	return temp;
}


/*
 * Create a copy of a filename with a ".old" suffix appended.
 */

#if	USE_PROTO
LOCAL CONST char * make_old (CONST char * name, CONST char * prev)
#else
LOCAL CONST char *
make_old (name, prev)
CONST char    *	name;
CONST char    *	prev;
#endif
{
	CONST char    *	temp;

	if (prev != NULL)
		return prev;

	if ((temp = malloc (strlen (name) + 5)) == NULL)
		throw_error (NO_MEMORY,
			     "cannot allocate pathname memory for %s.old",
			     name);
	sprintf (temp, "%s.old", name);
	return temp;
}


#define	SHORT_OPTIONS		"WrdtM:S:m:s:I:X:Y:Z:x:y:"

enum {
	WRITE_CHANGES		= 'W',
	REPORT_ONLY		= 'r',

	CONFIGURE_DEVICES	= 'd',
	CONFIGURE_TUNABLE	= 't',

	NEW_MDEVICES		= 'M',
	NEW_SDEVICES		= 'S',
	NEW_MTUNE		= 'm',
	NEW_STUNE		= 's',

	MDEVICE_NAME		= 'X',
	SDEVICE_NAME		= 'Y',
	TEMPLATE_NAME		= 'Z',
	MTUNE_NAME		= 'x',
	STUNE_NAME		= 'y',

	CONF_DIR		= 'I'
};



/*
 * Main program
 */

#if	USE_PROTO
int main (int argc, char * argv [])
#else
int
main (argc, argv)
int		argc;
char	      *	argv [];
#endif
{
	extinfo_t     *	extinfop;
	int		do_devices = 0;
	int		do_tune = 0;
	int		write_changes = 0;
	mdev_t	      *	mdev_changes = NULL;
	sdev_t	      *	sdev_changes = NULL;
	mtune_t	      *	mtune_changes = NULL;
	stune_t	      *	stune_changes = NULL;
	CONST char    *	mdev_name = "mdevice";
	CONST char    *	sdev_name = "sdevice";
	CONST char    *	mtune_name = "mtune";
	CONST char    *	stune_name = "stune";
	CONST char    *	template_name = "template.mak";
	CONST char    *	mdev_new = NULL;
	CONST char    *	sdev_new = NULL;
	CONST char    *	mtune_new = NULL;
	CONST char    *	stune_new = NULL;
	CONST char    *	mdev_old = NULL;
	CONST char    *	sdev_old = NULL;
	CONST char    *	mtune_old = NULL;
	CONST char    *	stune_old = NULL;
	int		i;

	register_name (argv [0]);
	symbol_init ();

	while ((i = getopt (argc, argv, SHORT_OPTIONS)) != -1)
		switch (i) {

		case  CONFIGURE_DEVICES:
			do_devices ++;
			mdev_new = make_new (mdev_name, mdev_new);
			sdev_new = make_new (sdev_name, sdev_new);
			mdev_old = make_old (mdev_name, mdev_old);
			sdev_old = make_old (sdev_name, sdev_old);
			break;

		case CONFIGURE_TUNABLE:
			do_tune ++;
			mtune_new = make_new (mtune_name, mtune_new);
			stune_new = make_new (stune_name, stune_new);
			mtune_old = make_old (mtune_name, mtune_old);
			stune_old = make_old (stune_name, stune_old);
			break;

		case NEW_MDEVICES:
			read_mdev_string (optarg, & mdev_changes);
			mdev_new = make_new (mdev_name, mdev_new);
			mdev_old = make_old (mdev_name, mdev_old);
			break;

		case NEW_SDEVICES:
			read_sdev_string (optarg, & sdev_changes);
			mdev_new = make_new (mdev_name, mdev_new);
			sdev_new = make_new (sdev_name, sdev_new);
			mdev_old = make_old (mdev_name, mdev_old);
			sdev_old = make_old (sdev_name, sdev_old);
			break;

		case NEW_MTUNE:
			read_mtune_string (optarg, & mtune_changes);
			mtune_new = make_new (mtune_name, mtune_new);
			mtune_old = make_old (mtune_name, mtune_old);
			break;

		case NEW_STUNE:
			read_stune_string (optarg, & stune_changes);
			mtune_new = make_new (mtune_name, mtune_new);
			stune_new = make_new (stune_name, stune_new);
			mtune_old = make_old (mtune_name, mtune_old);
			stune_old = make_old (stune_name, stune_old);
			break;

		case CONF_DIR:
			if (chdir (optarg) == 0)
				break;

			throw_error (CANNOT_UPDATE,
				     "Cannot change directory to %s, "
				     "OS says %s", optarg,
				     strerror (errno));
			return EXIT_FAILURE;

		case WRITE_CHANGES:
			write_changes ++;
			break;

		case REPORT_ONLY:
			_report_mode ++;
			break;

		case MDEVICE_NAME:
			mdev_name = optarg;
			break;

		case SDEVICE_NAME:
			sdev_name = optarg;
			break;

		case MTUNE_NAME:
			mtune_name = optarg;
			break;

		case STUNE_NAME:
			stune_name = optarg;
			break;

		case TEMPLATE_NAME:
			template_name = optarg;
			break;

		default:
			usage (argv [0]);
			return EXIT_FAILURE;
		}

	
	if (argc == 1 || optind != argc) {

		printf ("argc = %d, optind = %d\n", argc, optind);
#if	0
		usage (argv [0]);
#endif
		return EXIT_FAILURE;
	}


	if (mdev_new) {
		if (mdev_changes == NULL || ! write_changes) {
			free (mdev_new);
			mdev_new = NULL;
		}
		read_mdev_file (mdev_name, mdev_new, & mdev_changes);
	}
	if (sdev_new) {
		if (sdev_changes == NULL || ! write_changes) {
			free (sdev_new);
			sdev_new = NULL;
		}
		read_sdev_file (sdev_name, sdev_new, & sdev_changes);
	}
	if (mtune_new) {
		if (mtune_changes == NULL || ! write_changes) {
			free (mtune_new);
			mtune_new = NULL;
		}
		read_mtune_file (mtune_name, mtune_new, & mtune_changes);
	}
	if (stune_new) {
		if (stune_changes == NULL || ! write_changes) {
			free (stune_new);
			stune_new = NULL;
		}
		read_stune_file (stune_name, stune_new, & stune_changes);
	}

	if (do_devices) {
		if ((extinfop = assign_imajors ()) != NULL) {

			write_conf_c ("conf.c", extinfop);
			free (extinfop);
		}

		write_link ("drvbld.mak", template_name);
	}

	if (do_tune)
		write_conf_h ("conf.h");

	if (mdev_new) {
		remove (mdev_old);
		if (rename (mdev_name, mdev_old) != 0 ||
		    rename (mdev_new, mdev_name) != 0)
			throw_error (CANNOT_UPDATE,
				     "Unable to update '%s' file with "
				     "new entries, OS says %s", mdev_name,
				     strerror (errno));
	}
	if (sdev_new) {
		remove (sdev_old);
		if (rename (sdev_name, sdev_old) != 0 ||
		    rename (sdev_new, sdev_name) != 0)
			throw_error (CANNOT_UPDATE,
				     "Unable to update '%s' file with "
				     "new entries, OS says %s", sdev_name,
				     strerror (errno));
	}
	if (mtune_new) {
		remove (mtune_old);
		if (rename (mtune_name, mtune_old) != 0 ||
		    rename (mtune_new, mtune_name) != 0)
			throw_error (CANNOT_UPDATE,
				     "Unable to update '%s' file with "
				     "new entries, OS says %s", mtune_name,
				     strerror (errno));
	}
	if (stune_new) {
		remove (stune_old);
		if (rename (stune_name, stune_old) != 0 ||
		    rename (stune_new, stune_name) != 0)
			throw_error (CANNOT_UPDATE,
				     "Unable to update '%s' file with "
				     "new entries, OS says %s", stune_name,
				     strerror (errno));
	}
	return EXIT_SUCCESS;
}
