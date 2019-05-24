# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
for i in $*
	do case $i in
		-ver) echo $0 version 2.1:: 2.1;exit;;
		-flags) echo $0 \[-flags\] \[-ver\] \[word1 \| \"phrase 1\" ...\];exit;;
		-*) echo unknown worduse flag $i;exit;;
	esac
done
if test $# = 0
then
	echo	'When you see the ">" sign, type the word you want information about.

When you wish to stop, type q.

If no definition appears, either your word is misspelled or it is  
not included in our word list.
'
	while echo -e ">\c";read word
		if test \( "$word" = 'q' -o "$word" = '.' \)
		then 	exit
		fi
		do
			wordcap=`echo $word|tr "[a-z]" "[A-Z]"|tr -d \"\' `
			sed -n "/ $wordcap:.*$/,/^\%$/p" $L/wordlist.d|sed "s/^\%$/ /"
		done
	else	echo 'If no definition appears for the word(s) you entered, either  
your word is misspelled or it is not included in our word list.
'
while test "$1"
	do
		wordcap=`echo $1|tr "[a-z]" "[A-Z]"|tr -d \"\'`
		sed -n "/ $wordcap:.*$/,/^\%$/p" $L/wordlist.d|sed "s/^\%$/ /"
		shift
	done
	exit
fi
