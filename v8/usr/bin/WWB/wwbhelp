# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
for i in $*
	do case $i in
		-ver) echo $0 version 2.0:: 2.0;exit;;
		-flags) echo $0 \[-ver\] \[-flags\] \[word or part of word ..\];exit;;
		-*) echo unknown wwbhelp flag $i;exit;;
	esac
done
if test $# = 0
then
	echo -e 	'When you see the ">" sign, type the word you wish to have
more information for.  When you wish to stop, type q.

If no information appears, either your word is misspelled or
it is not included in the help file.\n'
	while echo -e ">\c";read word
		if test \( "$word" = 'q' -o "$word" = '.' -o "$word" = '' \)
		then 	exit
		fi
		do
			worduncap=`echo $word|tr [A-Z] [a-z]`
                        grep "$worduncap" $L/helplist.d
		done
else 	echo -e 'If no information appears, either your word is misspelled or
it is not included in the help file.\n'
	while test "$1"
		do
        		worduncap=`echo $1|tr [A-Z] [a-z]`
                        grep "$worduncap" $L/helplist.d
			shift
		done
	exit
fi
