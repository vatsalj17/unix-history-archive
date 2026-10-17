/*
 * Configurable information for the console driver(s).
 */

#define __KERNEL__	 1

#include <sys/kb.h>
#include "conf.h"

int		kb_lang = kb_lang_us;

/* Number of virtual console sessions on monochrome display. */
int		mono_count = MONO_COUNT;

/* Number of virtual console sessions on color display. */
int		vga_count = VGA_COUNT;

/* Greek keyboard option in vtnkb - 1=enabled, 0=disabled. */
int		VTGREEK = VTGREEK_SPEC;

/* Is separate shift status kept for different virtual console sessions? */
int		sep_shift = SEP_SHIFT;

/* Is console beeping enabled?  0=silent, 1=beep. */
int		con_beep = CON_BEEP_SPEC;
