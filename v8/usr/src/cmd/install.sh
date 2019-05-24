cmd=/bin/mv
case $1 in
	-s )	/usr/bin/strip $2
		shift
		;;
	-c )	cmd=cp
		shift
esac

if [ ! ${2-""} ]
then	echo 'install: no destination specified.'
	exit 1
fi

if [ -d $2 ]
then	file=$2/$1
else	file=$2
fi
rm -f $file
$cmd $1 $file
chmod o-w,g+w $file
if [ "`getuid`" = root ]
then	chgrp bin $file
	chown bin $file
fi
