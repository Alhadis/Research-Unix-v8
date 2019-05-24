# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
for i in $*
do case $i in
	-ver) echo $0 version 2.1: 2.0;exit;;
	-flags) echo $0 \[-flags\] \[-ver\] \[file ...\];exit;;
	-*) echo unknown chunk flag $i;exit;;
	*) if test ! -r $i
	   then echo Can\'t find the file $i\; try specifying a more complete pathname.; exit
	   fi
   esac
done
echo $*
deroff $*|sed -f $L/chkin.sed |$L/chunk |sed -f $L/chkout.sed
