#! /bin/sh
#
# @(#)ypinit.sh 1.1 85/05/30 Copyr 1984 Sun Microsystems, Inc.  
#
# ypinit.sh - set up a populated yp directory structure on a master server
# or a slave server.
#

# set -xv
yproot_dir=/etc/yp
hf=/tmp/ypinit.hostlist.$$

masterp=F
slavep=F
host=""
def_dom=""
master=""
got_host_list=F
exit_on_error=F
errors_in_setup=F

PATH=$PATH:$yproot_dir
export PATH 

case $# in
1)	case $1 in
	-m)	masterp=T;;
	*)	echo 'usage:'
		echo '	ypinit -m'
		echo '	ypinit -s master_server'
		echo ""
		echo "\
where -m is used to build a master yp server data base, and -s is used for"
		echo "\
a slave data base.  master_server must be an existing reachable yp server."
		exit 1;;
	esac;;

2)	case $1 in
	-s)	slavep=T; master=$2;;
	*)	echo 'usage:'
		echo '	ypinit -m'
		echo '	ypinit -s master_server'
		echo ""
		echo "\
where -m is used to build a master yp server data base, and -s is used for"
		echo "\
a slave data base.  master_server must be an existing reachable yp server."
		exit 1;;
	esac;;

*)	echo 'usage:'
	echo '	ypinit -m'
	echo '	ypinit -s master_server' 
	echo ""
	echo "\
where -m is used to build a master yp server data base, and -s is used for"
	echo "\
a slave data base.  master_server must be an existing reachable yp server."
	exit 1;;
esac


host=`hostname`

if [ $? -ne 0 ]
then 
	echo "Can't get local host's name.  Please check your path."
	exit 1
fi

if [ -z $host ]
then
	echo "The local host's name hasn't been set.  Please set it."
	exit 1
fi

def_dom=`domainname`

if [ $? -ne 0 ]
then 
	echo "Can't get local host's domain name.  Please check your path."
	exit 1
fi

if [ -z $def_dom ]
then
	echo "The local host's domain name hasn't been set.  Please set it."
	exit 1
fi

domainname $def_dom

if [ $? -ne 0 ]
then 
	echo "\
You have to be the superuser to run this.  Please log in as root."
	exit 1
fi

if [ ! -d $yproot_dir -o -f $yproot_dir ]
then
    echo "\
The directory $yproot_dir doesn't exist.  Restore it from the distribution."
	exit 1
fi

if [ $slavep = T ]
then
	if [ $host = $master ]
	then
		echo "\
The host specified should be a running master yp server, not this machine."
		exit 1
	fi
fi

echo "Installing the yp data base will require that you answer a few questions."
echo "Questions will all be asked at the beginning of the procedure."
echo ""
echo -n "Do you want this procedure to quit on non-fatal errors? [y/n: n]  "
read doexit

case $doexit in
y*)	exit_on_error=T;;
Y*)	exit_on_error=T;;
*)	echo "\
OK, please remember to go back and redo manually whatever fails.  If you"
	echo "\
don't, some part of the system (perhaps the yp itself) won't work.";;
esac

echo ""

for dir in $yproot_dir/$def_dom $yproot_dir/yp_private
do

	if [ -d $dir ]
	then
	echo -n "Can we destroy the existing $dir and its contents? [y/n: n]  "
		read kill_old_dir

		case $kill_old_dir in
		y*)	rm -r -f $dir

			if [ $?  -ne 0 ]
			then
			echo "Can't clean up old directory $dir.  Fatal error."
				exit 1
			fi;;

		Y*)	rm -r -f $dir

			if [ $?  -ne 0 ]
			then
			echo "Can't clean up old directory $dir.  Fatal error."
				exit 1
			fi;;

		*)    echo "OK, please clean it up by hand and start again.  Bye"
			exit 0;;
		esac
	fi

	mkdir $dir

	if [ $?  -ne 0 ]
	then
		echo "Can't make new directory $dir.  Fatal error."
		exit 1
	fi

done

if [ $slavep = T ]
then

	echo "\
There will be no further questions. The remainder of the procedure should take"
	echo "a few minutes, to copy the data bases from $master."

	for dir in $yproot_dir/yp_private $yproot_dir/$def_dom
	do
		chmod go+w $dir

		su daemon -c "/usr/ucb/rcp $master:$dir/*.pag $dir"

		if [ $?  -ne 0 ]
		then
			echo "Failed to rcp *.pag from $master:$dir."
			errors_in_setup=T

			if [ $exit_on_error = T ]
			then
				exit 1
			fi
		fi

		su daemon -c "/usr/ucb/rcp $master:$dir/*.dir $dir"

		if [ $?  -ne 0 ]
		then
			echo "Failed to rcp *.dir from $master:$dir."
			errors_in_setup=T

			if [ $exit_on_error = T ]
			then
				exit 1
			fi
		fi

		chmod go-w $dir
		chown root $dir/*

	done

	echo ""
	echo -n "${host}'s yellowpages data base has been set up"


	if [ $errors_in_setup = T ]
	then
		echo " with errors.  Please remember"
		echo "to figure out what went wrong, and fix it."
	else
		echo " without any errors."
	fi

	echo ""
	echo "\
At this point, make sure that /etc/passwd, /etc/hosts, /etc/networks,"
	echo "\
/etc/group, /etc/protocols, /etc/services/ and /etc/netgroup have been "
	echo "\
edited so that when the yellow pages is activated, the data bases you"
	echo "\
have just created will be used, instead of the /etc ASCII files."

	exit 0
else

	rm -f $yproot_dir/*.time

	while [ $got_host_list = F ]
	do
		echo $host >$hf
		echo ""
		echo "\
At this point, we have to construct a list of the hosts which will run yp"
		echo "\
servers.  $host is in the list of yp server hosts.  Please continue to add"
		echo "\
the names for the other hosts, one per line.  When you are done with the"
		echo "list, type a <ctl D>."
		echo "	next host to add:  $host"
		echo -n "	next host to add:  "

		while read h
		do
			echo -n "	next host to add:  "
			echo $h >>$hf
		done

		echo ""
		echo "The current list of yp servers looks like this:"
		echo ""

		cat $hf
		echo ""
		echo -n "Is this correct?  [y/n: y]  "
		read hlist_ok

		case $hlist_ok in
		n*)	got_host_list=F
			echo "Let's try the whole thing again...";;
		N*)	got_host_list=F
			echo "Let's try the whole thing again...";;
		*)	got_host_list=T;;
		esac
	done

	echo "\
There will be no further questions. The remainder of the procedure should take"
	echo "5 to 10 minutes."

	echo "Building $yproot_dir/yp_private/ypservers..."
	$yproot_dir/makedbm $hf $yproot_dir/yp_private/ypservers

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/yp_private/ypservers."
		errors_in_setup=T

		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	rm $hf

	echo "Copying ypservers to $yproot_dir/$def_dom..."

	for ext in dir pag
	do
		cp $yproot_dir/yp_private/ypservers.$ext $yproot_dir/$def_dom

		if [ $?  -ne 0 ]
		then
			echo "\
Couldn't cp $yproot_dir/yp_private/ypservers.$ext to $yproot_dir/$def_dom"
			errors_in_setup=T
		
			if [ $exit_on_error = T ]
			then
				exit 1
			fi
		fi

	done

	in_pwd=`pwd`
	cd $yproot_dir/yp_private

	echo "Building $yproot_dir/yp_private/ypdomains..."
	$yproot_dir/makedbm - ypdomains <<+
yp_private	yp_private
$def_dom	$def_dom
+

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/yp_private/ypdomains./"
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	echo "Building $yproot_dir/yp_private/ypmaps..."
	$yproot_dir/makedbm - ypmaps <<+
ypdomains	$host
ypmaps	$host
ypservers	$host
hosts.byname	$host
+

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/yp_private/ypmaps."
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	echo "Building $yproot_dir/yp_private/hosts.byname..."
	cat /etc/hosts | sed -e "/^#/d" -e s/#.*$// $h |\
	    awk '{for (i = 2; i <= NF; i++) print $i, $0}' |\
	    makedbm - $yproot_dir/yp_private/hosts.byname

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/yp_private/hosts.byname"
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	cd $yproot_dir/$def_dom

	echo "Building $yproot_dir/$def_dom/ypmaps..."
	$yproot_dir/makedbm - ypmaps <<+
ypmaps	$host
ypservers	$host
hosts.byname	$host
hosts.byaddr	$host
passwd.byname	$host
passwd.byuid	$host
group.byname	$host
group.bygid	$host
netgroup	$host
networks.byaddr	$host
networks.byname	$host
protocols.byname	$host
protocols.bynumber	$host
services.byname	$host
+

	if [ $?  -ne 0 ]
	then
		echo "Couldn't build yp data base $yproot_dir/$def_dom/ypmaps."
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	cd $yproot_dir
	echo "Building $yproot_dir/$def_dom/passwd.by*..."
	make NOPUSH=1 passwd > /dev/null

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/$def_dom/passwd.by*."
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	echo "Building $yproot_dir/$def_dom/hosts.by*..."
	make NOPUSH=1 hosts > /dev/null

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/$def_dom/hosts.by*."
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	echo "Building $yproot_dir/$def_dom/group.by*..."
	make NOPUSH=1 group > /dev/null

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/$def_dom/groups.by*."
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	echo "Building $yproot_dir/$def_dom/networks.by*..."
	make NOPUSH=1 networks > /dev/null

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/$def_dom/networks"
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	echo "Building $yproot_dir/$def_dom/netgroup..."

	if [ ! -f /etc/netgroup ]
	then 
		echo > /etc/netgroup
	fi

	make NOPUSH=1 netgroup > /dev/null

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/$def_dom/netgroup"
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	echo "Building $yproot_dir/$def_dom/services.by*..."
	make NOPUSH=1 services > /dev/null

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/$def_dom/services"
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	echo "Building $yproot_dir/$def_dom/protocols.by..."
	make NOPUSH=1 protocols > /dev/null

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/$def_dom/protocols.by..."
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	cd $in_pwd

	echo ""
	echo -n "\
$host has been set up as a yp master server"

	if [ $errors_in_setup = T ]
	then
		echo " with errors.  Please remember"
		echo "to figure out what went wrong, and fix it."
	else
		echo " without any errors."
	fi

	echo ""
	echo "\
If there are running slave yp servers, run yppush now for any data bases"
	echo "\
which have been changed.  If there are no running slaves, run ypinit on"
	echo "\
those hosts which are to be slave servers."

fi
