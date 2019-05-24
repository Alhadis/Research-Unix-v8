# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
trap 'rm -f /tmp/$$; trap 0;exit' 0 1 2 3 15
file=
fflag=
bflag=
vflag=
xflag=
for i in $*
	do case $i in
		-x) xflag=-x;shift; continue;;
		-v) vflag=-v;shift;continue;;
		-b) bflag=-b;shift;continue;;
		-f) fflag=F;shift;file=$1;shift;continue;;
		-ver) echo $0 version 2.0;exit;;
		-flags) echo $0 \[-f wfile\] \[-b\] \[-v\] \[-ver\] \[-flags\] \[file ...\];exit;;
		-*) echo unknown spellwwb flag $i;exit;;
		*) if test ! -r $i
		   then echo Can\'t find the file $i\; try specifying a more complete pathname.; exit
		   fi
	esac
done
if test $fflag
then 	if test ! -r $file
	then echo Can\'t find your file $file\; try specifying a more complete pathname.
		exit 1
	else spell $bflag $xflag $vflag $*|sort|comm -23 - $file >/tmp/$$
	fi
else 	if test -r $HOME/lib/spelldict 
	then spell  $bflag $xflag $vflag $* |sort|comm -23 - $HOME/lib/spelldict>/tmp/$$
	else spell  $bflag $xflag $vflag $*>/tmp/$$
	fi
fi
if test -s /tmp/$$ 
then echo -e Possible spelling errors in  $* are:"\n"
	pr -3 -t -i" "25 /tmp/$$
	echo -e '\nIf any of these words are spelled correctly, later type
                  spelladd word1 word2 ... wordn
to have them added to your spelldict file.'
fi
