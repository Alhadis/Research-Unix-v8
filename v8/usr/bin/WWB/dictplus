# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
trap 'rm -f /tmp/$$*; trap 0;exit' 0 1 2 3 15
nflag=
fflag=
file=
for i in $*
	do case $i in
		-f) fflag=-f; val=f; shift; file=$1;shift;continue;;
		-n) nflag=-n;shift;continue;;
		-ver) echo $0 version 2.2: 2.0: 2.1,2.0;exit;;
		-flags) echo $0 \[-f pfile\] \[-n\] \[-flags\] \[-ver\] \[file ...\];exit;;
		-*) echo unknown dictplus flag $i;exit;;
		*) if test ! -r $i
		   then echo Can\'t find the file $i\; try specifying a more complete pathname.; exit
		   fi
	esac
done
echo $*
if test $val
then 	if test ! -r $file
	then echo Can\'t find the pattern file $file.
		exit 1
	fi
fi
$L/dprog -l $nflag $fflag $file  -o /tmp/$$dictsed $*
if test -s /tmp/$$dictsed
then
	echo -e  '\nPlease wait for the substitution phrases\n
\n-------------------   Table of Substitutions   --------------------\n
PHRASE                     SUBSTITUTION\n'
	 sort -u /tmp/$$dictsed|sed -f $L/script.sed|split -50 -f /tmp/$$d_
	for i in /tmp/$$d_*
	do
		sed -n -f $i $L/suggest.d >>/tmp/$$d
	done
	sort /tmp/$$d
	echo -e '\n------------------------------------------------------------------\n'
fi
