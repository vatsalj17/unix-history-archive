#! /bin/sh
#
#	makemailids.sh
#
#	Generates the mailids and users files from /etc/passwd.
#	/etc/passwd is expected to contain an "<mailid>" entry in
#	the GCOS field.  This field may eventually contain more than
#	one mailid.
#
PATH=/bin:/usr/bin:.

ed - /etc/passwd <<DONE
v/</d
g/:.*</s//	/
g/>.*/s///
w users.new
g/\(.*\)	\(.*\)/s//\2	\1/
w mailids.new
q
DONE
mv users users.bak
mv users.new users
mv mailids mailids.bak
mv mailids.new mailids
