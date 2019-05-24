# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
for i in $*
	do case $i in
		-ver) echo $0 version 2.1;exit;;
		-flags) echo $0 style-file1 \[style-file2\] \[-flags\] \[-ver\];exit;;
		-*) echo unknown match flag $i;exit;;
		*) if test ! -r $i
		   then echo Can\'t find the file $i\; try specifying a more complete pathname.; exit
		   fi
	esac
done
echo "            READABILITY"
grep Kin $*
echo
echo "            TOTAL LENGTH"
grep "no\. sent" $*
echo
echo "            SENTENCE LENGTH"
grep "sent leng" $*
echo
echo "            QUESTIONS"
grep question $*
echo
echo "            SIMPLE SENTENCES"
grep simple $*
echo
echo "            TOBE USE"
grep tobe $*
echo
echo "            PASSIVES"
grep passive $*
echo
echo "            NOUN-PRONOUN USE"
grep "noun.*adj.*pron" $*
echo
echo "            NOMINALIZATIONS"
grep nominaliz $*
