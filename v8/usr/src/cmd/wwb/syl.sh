# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
if test $# = 0
then $L/syl;exit
fi
n=
for i in $*
	do case $i in
		-ver) echo $0 version 2.0: 2.0;exit;;
		-flags) echo $0 \[-num\] \[-flags\] \[-ver\] \[file ...\];exit;;
		-*) n=$1;shift;continue;;
		*) if test ! -r $i
		   then echo Can\'t find the file $i\; try specifying a more complete pathname.
			   exit 1
		   fi
	esac
done
if test $# != 0
then	deroff -w $*|tr "[A-Z]" "[a-z]"|$L/syl $n|sort -u
else $L/syl $n; exit
fi
