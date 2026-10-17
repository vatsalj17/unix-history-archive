13-Sep-84 14:21:49-EDT,2535;000000000001
Mail-From: SY.FDC created at 13-Sep-84 14:21:45
Date: Thu 13 Sep 84 14:21:44-EDT
From: Frank da Cruz <SY.FDC@CU20B.ARPA>
Subject: Macintosh Kermit #1 Available
To: Info-Kermit@CU20B.ARPA, Info-Mac@SUMEX-AIM.ARPA
cc: engel@HARVARD.ARPA

This is to announce Macintosh Kermit #1.  It's one of several different
Macintosh Kermits under development, but this was the first one to reach us.
It's from Stephen Engel at Harvard University (engel@harvard), written in C for
the Sumacc Unix-based Macintosh cross development system.  The files are
collected together into a Unix shell archive, as MC1KERMIT.SH.  They include
source files, a makefile, and some sketchy documentation.

In Steve's words, "This is an `alpha' release of the program.  Much of the code
is sloppy and buggy, but I believe that all the program's funadmentals work
adequately.  This program is chiefly an adaptation of Columbia's Unix kermit."

It has been tested in conjunction with Unix Kermit and DEC-20 Kermit.  The
program is far from complete and has many rough edges, but it's the first out
the door and so should prove useful to the many sites that need some kind of
Kermit capability for the Mac.

You must have the Sumacc cross development system and a VAX-resident 68000
C cross compiler to make use of these files (anybody out there want to make
a hex file suitable for use with the FROMHEX utility?).  To get MacKermit
onto a Mac, you must:

1. Get MC1KERMIT.SH to your VAX Unix system, preferably in its own directory.
2. Connect your Mac to the VAX using Macterminal 0.5 or later.
3. Be sure your search path includes the Sumacc tools.
4. CD to the directory where you've put the mc1kermit.sh file.
5. Type "sh < mc1kermit.sh" to let the shell pick the file apart.
6. Type "make" to build the program from the source.
7. Type "macput -r kermit" to send the Kermit program to the Mac.
8. On the Mac, use SetFile to set the "bundle bit" (optional).

The VAX directory will contain the various source and header files, as well
as the Mac-format resource file, and a brief document "kermitdoc", explaining
the operation of this version of Macintosh Kermit.

The shell archive file is available as KER:MC1KERMIT.SH on COLUMBIA-20 and
CU20B via anonymous FTP.  Comments, suggestions, bug reports should be
directed to engel@harvard with cc to Info-Kermit@CU20B (or @COLUMBIA-20 if
your host doesn't know how to mail to CU20B yet).

Thanks, Steve, and congratulations on being the first to do it.  - Frank
-------
