#include "hd.h"
#include "classify.h"
#include "mydir.h"

#define aout1	0407
#define aout2	0410
#define aout3	0411
#define aout4	0405
#define	cpio	070707
#define ar	0177545

/* Classify return the file type of parameter fname */

classify (fname) char * fname; {

	int fdesc, rdlen, word;
	unsigned mode;

	char *lastfn();

	if (stat (fname, &scr_stb)) return CL_NULL;

	mode = scr_stb.st_mode & S_IFMT;
	if (mode == S_IFDIR) return CL_DIR;
	if (mode != S_IFREG) return CL_SPCL;

	fdesc = open (fname, 0);
	if (fdesc < 0) return CL_PROTPLN;

	rdlen = read (fdesc, &word, sizeof word);
	close (fdesc);
	if (rdlen < sizeof word) return CL_TEXT;

	if (word == aout1 || word == aout2 || word == aout3 ||
		word == aout4) return CL_AOUT;
	
	if (word == ar) return CL_AR;
	if (word == cpio) return CL_CPIO;

	if (compe (lastfn (fname), "core")) return CL_CORE;

	return CL_TEXT;
}

/* Lastfn returns a pointer to the last file name in path.  */

char *
lastfn (path) register char *path; {
	register char *cp;

	for (cp=path; *cp++;);

	cp--;
	while (*--cp != '/' && cp >= path);

	return ++cp;
}
