# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
audflag=m
for i in $*
	do case $i in
		-tm) audflag=m;shift;break;;
		-t) audflag=t;shift;break;;
		-c) audflag=c;shift;break;;
		-x) audflag=x;shift;file=$1;shift;break;;
		-ver) echo $0 version 2.1:: 2.0;exit;;
		-flags) echo $0 \[-t \| -c \| -tm \| -x standards-file\] \[-flags\] \[-ver\];exit;;
		-*) audflag=q;echo unknown wwbstand flag $i;shift;exit;;
		*) audflag=;echo "wwbstand takes only flags [-t|-tm|-c|-x file]";shift;exit;;
	esac
done

if test $audflag = m
then echo -e 'These are desirable ranges for technical documents based
on 30 TMs judged to be good by department heads in the Research Area:\n'
	$L/standlkup < $L/tm.st
	exit

elif test $audflag = t
then echo -e 'These are desirable ranges for training documents derived
from 34 instructional texts produced by Dept. 45272:\n'
	$L/standlkup < $L/train.st
	exit

elif test $audflag = c
then echo -e 'These are desirable ranges for documents for craft based on
research of Esther Coke in Dept. 11222.'
	$L/standlkup < $L/crft.st
	exit

elif test $audflag = x
then echo -e "Your file $file contains the following standards:\n"
	$L/standlkup < $file
	exit
fi
