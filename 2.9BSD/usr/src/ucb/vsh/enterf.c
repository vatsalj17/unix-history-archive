#include "hd.h"
#include "mydir.h"
#include "classify.h"

/* Enterfile selects its parameter fname;  Fname may either be a single
   element file name (no slashes) or a full path name.  Enterfile
   can handle files which look like ".", "..", "file.c", or "/a/b".
   For files which could look like "/a/b/", "a/b", or "./../../a" use
   the procedure "file" (file expects an argument in argv format).
*/
enterfile (fname) char *fname; {

switch (classify (fname)) {

	case CL_NULL:  case CL_PROTPLN:
	myperror (fname);  return FAILURE;

	case CL_DIR:
	return enterdir (fname);

	case CL_CORE:
	f_exec (DEBUGGER, DEBUGGER, fname, 0); return SUCCESS;

	case CL_AOUT:
	p_exec ("/bin/nm", "nm", fname, 0);  return REPLOT;

	case CL_AR:
	p_exec ("/bin/ar", "ar", "vt", fname, 0);  return REPLOT;

	case CL_CPIO:
	p_exec ("/bin/cpio", "cpio", "-vt", 0);  return REPLOT;

	case CL_TEXT:
	f_exec (EDITOR, EDITOR, fname, 0);  return REPLOT;

	default:
	putmsg (fname);  printf (":  Vsh cannot handle this file");
}
return FAILURE;
}
home () {			/* enter home directory */

	return file (&envhome);
}
