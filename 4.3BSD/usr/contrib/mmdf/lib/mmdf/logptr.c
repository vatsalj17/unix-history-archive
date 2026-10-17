#include "util.h"
#include "mmdf.h"

/*
	this is a surrogate declaration of logptr.  most of the
	mmdf modules use "logptr" as the generic reference to the
	log structure.  most mmdf main modules bind logptr to
	msg.log or chan.log or somesuch.  

	this file will bind it to chan.log.

*/

extern LLog chanlog;
LLog *logptr = &chanlog;
