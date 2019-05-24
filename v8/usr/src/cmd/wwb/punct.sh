B=/usr/bin
L=/usr/lib/style
trap 'rm -f /tmp/$$* ; exit'  1 2 3 15
a=$*
if test $# = 0
then
a=-
cat >/tmp/$$a
$B/double /tmp/$$a
else
$B/double $*
fi
echo
for i in $a
do case $i in
		-) :;;
		-*) echo unknown punct flag $i; exit;;
		*) if test ! -r $i
			then echo Can\'t find the file $i\; try specifying a more complete pathname.
				exit;
			fi
	esac
	if test $i = '-'
	then
	sed "s/^'/./" /tmp/$$a >/tmp/$$in
	else
	sed "s/^'/./"  $i >/tmp/$$in
	fi
	$L/punlx </tmp/$$in >/tmp/$$pu
	diff /tmp/$$in /tmp/$$pu >/tmp/$$diff
	if test ! -s /tmp/$$diff
	then 
		:
	else echo $i
	 	sed -f $L/seddiff /tmp/$$diff 
		echo
	fi
	rm -f /tmp/$$*
	if test $i = '-'
	then exit;
	fi
	shift;continue;
done
