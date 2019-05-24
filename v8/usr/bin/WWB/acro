# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
trap 'rm -f /tmp/$$*; trap 0; exit' 0 1 2 3 15
for i in $*
do case $i in
	-ver) echo $0 version 2.0: 2.0;exit;;
	-flags) echo $0 \[-s\] \[-flags\] \[-ver\] file ...;exit;;
	-*) echo unknown acro flag $i;exit;;
        *) if test ! -r $i
        then    echo -e \\nCan\'t find the text file $i\; try specifying a more complete pathname.; exit
        fi
   esac
done
echo "This program searches for acronyms in a text file.
It will also find words that are printed in capital letters."
for i in $*
do
	deroff -w $i | grep "[A-Z][A-Z]" |
	sort -d  | uniq -c > /tmp/$$2
	if test -s /tmp/$$2
	then
		echo -e "\nThe following acronyms are used in file \"$i\":"
		cat /tmp/$$2
		echo -e "\nAcronyms appear on the following lines of \"$i\":\n"
		 sed -f $L/acro.sed /tmp/$$2 > /tmp/$$3
		$L/dprog -A -n -l -f /tmp/$$3 $i
	else echo -e "\nNo acronyms found in file \"$i\".\n"
	fi
done
