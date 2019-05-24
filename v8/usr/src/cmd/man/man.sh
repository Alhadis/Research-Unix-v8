#!/bin/sh
MAN=/usr/man
CACHE=/usr/spool/man
cmd= sec= fil= opt= i= all= quick=
cmd=q sec=\?

if [ -d $MAN ]
then	cd $MAN
	cacheonly=
else	cd $CACHE
	cacheonly=y
fi

for i
do	case $i in
	[1-9])		sec=$i ;;
	-q)		cmd=q ;;
	-n)		cmd=n ;;
	-t)		cmd=t ;;
	-k)		cmd=k ;;
	-e | -et | -te) cmd=e ;;
	-ek | -ke)	cmd=ek ;;
	-ne | -en)	cmd=ne ;;
	-w)		cmd=where ;;
	-*)		opt="$opt $i" ;;
	*)		fil=`echo man$sec/$i.*`
			case $fil in
			man7/eqnchar.7)	all="$all /usr/pub/eqnchar $fil" ;;
			*\*)		echo $i not found 1>&2 ;;
			*)		if [ "$cacheonly" = y ]
					then	quick="$quick $fil"
					elif [ "$cmd" = q ]
					then	qf=$CACHE/$fil
						if newer $qf $fil
						then	quick="$quick $qf"
						else	all="$all $fil"
						fi
					else	all="$all $fil"
					fi ;;
			esac
	esac
done

case "$all$quick" in
	"") exit ;;
esac

if [ "$cacheonly" = y -a "$cmd" != q ]
then	echo unformatted manual pages are not available on this system 1>&2
	exit
fi

case $cmd in

q)		if [ -t ]
		then (	if [ "$quick" != "" ]
			then	cat $quick
			fi
			if [ "$all" != "" ]
			then	nroff -man $all
			fi) | ul | sed ':x
				/^$/{
					N
					s/^\n$//
					bx
				}'
		else 	if [ "$quick" != "" ]
			then	cat $quick
			fi
			if [ "$all" != "" ]
			then	nroff -man $all
			fi
		fi ;;
n)		nroff $opt -man $all ;;
ne)		neqn $all | nroff $opt -man ;;
t)		troff $opt -man $all ;;
k)		troff -t $opt -man $all | tc ;;
e)		eqn $all | troff $opt -man ;;
ek)		eqn $all | troff -t $opt -man | tc ;;
where)		echo $all ;;

esac
