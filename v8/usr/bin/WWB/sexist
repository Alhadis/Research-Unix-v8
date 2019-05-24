# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
trap ' rm -f /tmp/$$*; trap 0;exit' 0 1 2 3 15
for i in $*
	do case $i in
		-ver) echo $0 version 2.3: 2.0: 2.1,2.0;exit;;
		-flags) echo $0 \[-flags\] \[-ver\] \[file ...\];exit;;
		-f) fflag=F; shift; file=$1; shift;continue;;
		-*) echo unknown sexist flag $i;exit;;
		*) 	if test ! -r $i
			then echo Can\'t find the file $i\; try specifying a more complete pathname.
				exit 1
			fi
	esac
done
echo $*
if test $fflag
then	cat $L/sexist.d $file >/tmp/$$phrase
	$L/dprog -l -n -f /tmp/$$phrase -o /tmp/$$sexsed $*
elif test -s $HOME/lib/sexdict
then
	echo -e Sexist is using your phrases in $HOME/lib/sexdict."\n"
	cat $L/sexist.d $HOME/lib/sexdict >/tmp/$$phrase
	$L/dprog -l -n -f /tmp/$$phrase -o /tmp/$$sexsed $*
else
	$L/dprog -l -n -f $L/sexist.d -o /tmp/$$sexsed $*
fi
if test -s /tmp/$$sexsed
then
 	echo -e '\nPlease wait for the substitution phrases\n
\n-------------------   Table of Substitutions   --------------------\n
PHRASE                     SUBSTITUTION\n'
	 sort -u /tmp/$$sexsed |sed -f $L/script.sed| split -50 -f /tmp/$$s_
for i in /tmp/$$s_*
do 
	sed -n -f $i $L/sexist.sg >>/tmp/$$s
done
sort /tmp/$$s
	echo -e '\n------------------------------------------------------------------\n'
fi
