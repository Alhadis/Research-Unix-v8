# run this as uid bin, gid bin

umask 2
MAN=/usr/man
CACHE=/usr/spool/man

cd $MAN
for sec in man[1-9]
do	if [ ! -d $CACHE/$sec ]
	then	mkdir $CACHE/$sec
	fi
	for src in $sec/*
	do	dest=$CACHE/$src
		if newer $src $dest
		then	date=`/usr/lib/man $src`
			e= t=

			if grep -s '^\.EQ *$' $src
			then	e=e
			fi

			if grep -s '^\.TS *$' $src
			then	t=t
			fi

			case "$e$t" in
			"")	nroff -man $date $src ;;
			e)	neqn /usr/pub/eqnchar $src | nroff -man $date ;;
			t)	tbl $src | nroff -man $date ;;
			et)	neqn /usr/pub/eqnchar $src | tbl | nroff -man $date
			esac >$dest
		fi
	done
done
