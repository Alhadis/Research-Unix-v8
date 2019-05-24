# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
trap 'rm -f /tmp/$$abst; trap 0; exit' 0 1 2 3 15;
mflag=-ms
mlflag=-ml
sflag=
fflag=
file=
xfile=
audflag=m
for i in $*
	do case $i in
		-f) fflag=F; shift; file=$1;shift;continue;;
		-mm) mflag=-mm;shift;continue;;
		-ms) mflag=-ms;shift;continue;;
		-ml|-li) mlflag=-ml;shift;continue;;
		+li) mlflag=;shift;continue;;
		-c) audflag=c;shift;continue;;
		-t) audflag=t;shift;continue;;
		-tm) audflag=m;shift;continue;;
		-x) audflag=x;shift; xfile=$1; shift;continue;;
		-s) sflag=s;shift;continue;;
		-ver) echo $0 version 2.0: 2.2: 2.0;exit;;
		-flags) echo $0 "[-flags] [-ver] [-tm|-c|-t|-x standards-file] [-mm|-ms] [-li|+li] [-s] [-f style-file|file ...]";exit 0;;
		-*) echo unknown prose flag $i; exit;;
		*) if test ! -r $i
		   then echo Can\'t find the file $i\; try specifying a more complete pathname.; exit
		   fi
	esac
done

if test $fflag 
then	if test ! -r $file 
	then echo Can\'t find the file $file, try specifying a more complete pathname.
		exit 1
	fi
	if test $sflag
	then $L/prose f${sflag}${audflag} $xfile  < $file 
	else $L/prose f${audflag} $xfile  < $file |pr -h "PROSE OUTPUT FOR $file" 
	fi
else
	if test $sflag
	then deroff $mflag $mlflag $* | $L/style1 | $L/style2 | $L/style3 | tee styl.tmp| $L/prose ${sflag}${audflag} $xfile $$ 
	else deroff $mflag $mlflag $* | $L/style1 | $L/style2 | $L/style3 | tee styl.tmp| $L/prose ${audflag} $xfile $$ |pr -h "PROSE OUTPUT FOR $*"
	fi
fi
