if test "$1" then
	if ! creatdb $2 $1 then exit fi
	ingres -s $2 $1 < /mnt/ingres/demo/qry
	echo	database $1 created
	exit
else
	echo no database name
fi
