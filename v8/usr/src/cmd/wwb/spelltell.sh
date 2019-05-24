# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
for i in $*
	do case $i in
	-ver) echo $0 version 2.0:: 2.0;exit;;
	-flags) echo $0 \[-flags\] \[-ver\] \[wordpart \| regular expression ...\];exit;;
	-*) echo unknown spelltell flag $i;exit;;
	esac
done
if test $# = 0
then 	echo Do you want instructions? \(y or n\)
	read ans
	if 	test \( $ans = 'y' -o $ans = 'Y' -o $ans = 'yes' -o $ans = 'Yes' \)
	then echo '
This command uses the grep command to search a dictionary 
for the word you are interested in.  When you get a ">" 
you can  type either:
        1) any contiguous letters that you are sure 
		of in the word
                example: for "accommodate" you could use:
                        ac
                        acc
                        accom
                        date, etc.
        2) a grep pattern specifying what you know of the word
                example: for accommodate
                        accomm.date
                        ^acc
                        accom*.date, etc.

After your last phrase, type q (for quit).
You can also run this program by typing:
'
echo "		spelltell 'phrase1'   'phrase2' ..."
	fi
	while echo -e ">\c"; read ans
	pat=`echo $ans|tr  -d \"\' `
	if test \( "$pat" = 'q' -o "$pat" = '.' -o "$pat" = '' \)
	then exit
	fi
	do grep $pat $L/spelllist.d
	done
else
	while test "$1"
		do
			grep $1 $L/spelllist.d
			shift
		done
	exit
fi
