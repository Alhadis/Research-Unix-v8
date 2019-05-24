# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style

trap 'rm -f /tmp/$$*; trap 0; exit' 0 1 2 3 15
if test $# = 0
then
	echo 'Usage: mkstand [-mm|-ms][-li|+li] [-o outfile] file1 file2 file3 ...'
	exit 2
fi
mflag=-ms
mlflag=-ml
for flag in $*
do case $flag in
	-mm) mflag=-mm;shift;continue;;
	-ms) mflag=-ms;shift;continue;;
	-li|-ml) mlflag=-ml;shift;continue;;
	+li|-tt) mlflag=;shift;continue;;
	-o) shift;outfile=$1;shift;continue;;
        -flags) echo "$0 [-mm|-ms][-li|+li][-o outfile][-ver][-flags] file1 file2 ..."; exit;;
	-ver) echo "mkstand version 2.0: 2.0";exit;;
	-*) echo unknown mkstand flag $flag;exit;;
		*) if test ! -r $flag
		   then echo Can\'t find the file $flag\; try specifying a more complete pathname.; exit
		   fi
esac
done
number=$#
if test $number -eq 1
then echo 'Mkstand requires more than one input file.
The standards will be most reliable if you include more than 20 input files.
Mkstand exits.'
	exit 2
fi
if test $number -lt 20
then 	echo 'There are fewer than 20 input files.  Mkstand will compute standards,
but they would be more reliable if you included at least 20
input files.'
fi
for text in $*
do
	style $mflag $mlflag $text >> /tmp/$$stat.out
done
$L/mkstand $number $$
if test \( $? -eq 0 -o $? -eq 2 \)
then	echo Mkstand exits.
	exit 2
fi
if test -z "$outfile"
then 	echo 'Mkstand is putting the standards it just compiled in
a file called "stand.out"
Use wwbstand to look at the standards in a readable way. Type
               wwbstand -x stand.out
Do not be concerned that some of the lines in stand.out are all zeros.
Those zeros are placeholders for variables that prose may consider someday.'
else	mv stand.out $outfile
      echo "Your standards are now in file $outfile.
Use wwbstand to look at the standards in a readable way. Type
               wwbstand -x $outfile
Do not be concerned that some of the lines in $outfile are all zeros.
Those zeros are placeholders for variables that prose may consider someday."
fi
