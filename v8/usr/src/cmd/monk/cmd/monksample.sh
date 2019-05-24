
#
# initalizations
#
case `uuname -l` in
	
mhuxm)
	MHOME=/m2/c1122/frodo/lib/monk
	;;
	
esac

#
# path names for macros and default references
#
MHOME=${MHOME-/usr/lib/monk}

#
# check the number of args
#
if [ $# != 1 ]
then
	cd $MHOME/sample
	echo -e "Usage: monksample \c" 1>&2
	preface="[ "
	for i in *
	do
		echo -e "$preface$i\c" 1>&2
		preface=" | "
	done

	echo " ]" 1>&2
	exit 1
fi

#
# produce the samples
#
cat $MHOME/sample/$1
