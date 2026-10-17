/* val_str.c */

#include "val_str.h"
#include "stune.h"

static char val_str_buf[STUNE_VAL_LEN] = "";
static int val_str_index;

/* Initialize value string area to empty string. */
void init_val_str()
{
	val_str_index = 0;
	val_str_buf[0] = '\0';
}

/* If it will fit, append a character to the value string buffer. */
void collect_val_str(c)
char c;
{
	if (val_str_index < STUNE_VAL_LEN -1) {
		val_str_buf[val_str_index++] = c;
		val_str_buf[val_str_index] = '\0';
	}
}

/* Return a pointer to the static value string buffer area. */
char * get_val_str()
{
	return val_str_buf;
}
