
#
# initalizations
#
arg0=tmonk

if [ "$CANDEST" ]
then
	candest="CANDEST=$CANDEST"
fi

if [ "$I10DEST" ]
then
	i10dest="I10DEST=$I10DEST"
fi

if [ $arg0 = tmonk ]
then
	case `uuname -l` in
	
	mhuxm)
		MHOME=/m2/c1122/frodo/lib/monk
		STDIN=-
		TERM=-Ti10
		eqn='eqn'
		i10send='i10send'
		nulloutput=
		output="|$i10dest i10send -von"
		roff='troff'
		;;
	
	alice)
		STDIN=/dev/stdin
		TERM=-Ti10
		eqn='eqn'
		i10send='/usr/frodo/bin/ui10send'
		nulloutput=
		output="|$i10dest /usr/frodo/bin/ui10send -von"
		roff='troff'
		;;
	
	araki)
		STDIN=/dev/stdin
		TERM=-Ti10
		eqn='eqn'
		i10send='ui10send'
		nulloutput=
		output='|ui10send -d5211out -von'
		roff='troff'
		;;
	
	*)
		STDIN=/dev/stdin
		TERM=-T202
		eqn='eqn'
		i10send='ui10send'
		nulloutput=
		output="|$candest dcan"
		roff='troff'
		;;
	
	esac

else
	STDIN=-
	TERM=-T${TERM-450}
	eqn='neqn'
	nulloutput=
	output=
	roff='nroff'
fi

#
# path names for macros and default references
#
MHOME=${MHOME-/usr/lib/monk}

MDB=${MDB-$MHOME/db}

EQN=/usr/pub/eqnchar

MP=${TMAC-$MHOME}/tmac.p

REFS=${REFS-/usr/frodo/refs/index}

PATH=$MHOME:$PATH
export PATH

#
# get options
#
for i
do
	case "$i" in

#
# troff, reference or output options
#
	-12)
		pitch=-12
		;;

	-A*)
		Acmd="|"`expr "$i" : '..\(.*\)'`
		;;

	-B*)
		Bcmd="|"`expr "$i" : '..\(.*\)'`
		;;

	-E)
		options="$options -e"
		;;

	-R*)
		REFS=`expr "$i" : '..\(.*\)'`
		;;

	-T202)
		TERM=$i
		output='|d202'
		;;

	-T9700)
		TERM=$i
		output='|dpr -mx'
		;;

	-Taps)
		TERM=$i
		output='|apsend'
		;;

	-Tcanon)
		i=-T202
		TERM=$i
		output="|$candest dcan"
		;;

	-Tcat)
		TERM=$i
		output='|gcat -f "$files"'
		;;

	-Timagen)
		i=-Ti10
		TERM=$i
		output="|$i10dest $i10send -von"
		;;

	-Tjerq)
		i=-T202
		TERM=$i
		output="|proof; cat /tmp/e$$; rm -f /tmp/e$$"
		exec 2> /tmp/e$$
		;;

	-Tlp)
		TERM=$i
		output='|lpr'
		;;

	-T-)
		output=$nulloutput
		;;

	-T*)
		TERM=$i
		;;

	-a)
		options="$options $i"
		roff=troff
		output=
		;;

	-x)
		verbose=on
		;;

	-*)
		preprocessors="$preprocessors $i"
		;;

	*)
		files="$files $i"
		;;

	esac
done

#
# find what pre processors monk needs
#
preprocessors=`
egrep -h '\|(source|remember|reference|equation|e|table|picture|ideal|ped|graph)[(<[{'\''"\`]|\|begin[(<[{'\''"\`](source|remember|reference|equation|e|table|picture|ideal|ped|graph)' $files |
awk '
/\|source[(<[{'\''"\`]/		{soelim++}
/\|begin[(<[{'\''"\`]source/	{soelim++}
/\|remember[(<[{'\''"\`]/	{cite++}
/\|begin[(<[{'\''"\`]remember/	{cite++}
/\|reference[(<[{'\''"\`]/	{refer++}
/\|begin[(<[{'\''"\`]reference/	{refer++}
/\|equation[(<[{'\''"\`]/	{eqn++}
/\|begin[(<[{'\''"\`]equation/	{eqn++}
/\|e[(<[{'\''"\`]/		{eqn++}
/\|begin[(<[{'\''"\`]e/		{eqn++}
/\|table[(<[{'\''"\`]/		{tbl++}
/\|begin[(<[{'\''"\`]table/	{tbl++}
/\|picture[(<[{'\''"\`]/	{pic++}
/\|begin[(<[{'\''"\`]picture/	{pic++}
/\|ideal[(<[{'\''"\`]/		{ideal++}
/\|begin[(<[{'\''"\`]ideal/	{ideal++}
/\|ped[(<[{'\''"\`]/		{tped++}
/\|begin[(<[{'\''"\`]ped/	{tped++}
/\|graph[(<[{'\''"\`]/		{grap++; pic++}
/\|begin[(<[{'\''"\`]graph/	{grap++; pic++}
END 				{
				 x = ""
				 if (soelim)  x = x " -s"
				 if (refer) x = x " -r"
				 if (cite)  x = x " -cn"
				 if (ideal) x = x " -i"
				 if (grap)  x = x " -g"
				 if (pic)   x = x " -p"
				 if (tped)  x = x " -tp"
				 if (tbl)   x = x " -t"
				 if (eqn)   x = x " -e"
				 print x
				}
'`" $preprocessors"

#
# get preprocessors and other options
#
for i in $preprocessors
do
	case "$i" in

#
# pre-processors
#
	-c)
		ccmd='|col'
		;;

	-cn)
		cncmd='|cite'
		erroutput='2>> .cite'
		;;

	-e)
		ecmd="|$eqn"
		;;

	-g)
		gcmd='|grap'
		;;

	-i)	icmd='|ideal -q'
		;;

	-p)
		pcmd='|pic'
		;;

	-r)
		rcmd="|refer -e -n -p $REFS |sed -f $MHOME/dorefer"
		;;

	-s)
		scmd='|soelim /dev/stdin'
		;;

	-t)
		tcmd='|tbl'
		;;

	-tp)
		tpcmd='|tped'
		;;

	-*)
		options="$options $i"
		;;

	esac
done


#
# special handeling for terminals
#
	if [ "$pitch" ]
	then
		case $TERM in

		-T300|-T300s|-T450|-T1620)
			TERM="$TERM-12"
			;;
		esac
	fi

	case $arg0  in

	tmonk)
		case $TERM in

		-Tcat|-Taps|-Ti10|-T202)
			;;


		-Tdover)
			TERM=
			;;

		*)
			echo 1>&2 Invalid $TERM
			exit 1
			;;

		esac
		;;

	nmonk)
		if [ "ccmd" ]
		then
			case $TERM in

			-T300|-T300s|-T450|-T4014|-Ttek|-T1620)
				ccmd="|col -f|greek $TERM"
				;;

			-T300-12|-T300s-12|-T450-12|-T1620-12)
				ccmd='|col -f|greek $TERM'
				options='$options -rW72'
				;;

			-T37|-T4000a|-T382|-TX)
				ccmd='|col -f'
				;;

			-Thp|-T2621|-T2640|-T2645)
				ccmd='|colcrt -'
				;;

			-T735|-T745)
				ccmd='|col -x'
				;;

			-T43)
				ccmd='|col -x'
				options='$options -rW75'
				;;

			-T40/4|-T40/2)
				ccmd='|col -b'
				;;

			esac
		fi

		case $TERM in

		-T1620)
			TERM=-T450
			;;

		-T1620-12)
			TERM=-T450-12
			;;

		-T37|-Ttn300|-T300S|-T300*|-T450*)
			;;

		*)
			TERM=
			;;

		esac
		;;

	*)
		echo 1>&2 This command must be called nmonk or troff
		exit 1
		;;

	esac

#
# special handleing for pic and eqn
#
if [ "$pcmd" ]
then
	macro="$macro $MP"

	case $TERM in

	-Ti10)
		pcmd="$pcmd -T240"
		;;

	-T202)
		pcmd="$pcmd -D"
		;;

	*)
		pcmd="$pcmd $TERM"
		;;

	esac
fi

if [ "$ecmd" ]
then
	case $TERM in

	-Ti10)
		ecmd="$ecmd -r240 -m6"
		;;

	-Taps)
		ecmd="$ecmd -Taps"
		;;

	-T202)
		ecmd="$ecmd -T202"
		;;

	esac

	ecmd="$ecmd -d $EQN $STDIN"
fi

#
# use default terminal for 202 so pic won't get confused with -T202
#
if [ "$TERM" = -T202 ]
then
	TERM=
fi

#
# do it
#
if [ $verbose ]
then
	echo 1>&2 "$MHOME/monk -d $MDB $files | sed -f $MHOME/dokludge $Bcmd \\
	$scmd $cncmd $rcmd $icmd $gcmd $tcmd $pcmd $tpcmd $ecmd $Acmd \\
	| $roff $TERM $options $macro - $erroutput $ccmd $output"
fi
exec sh -c "exec $MHOME/monk -d $MDB $files | sed -f $MHOME/dokludge $Bcmd \\
	$scmd $cncmd $rcmd $icmd $gcmd $tcmd $pcmd $tpcmd $ecmd $Acmd \\
	| $roff $TERM $options $macro - $erroutput $ccmd $output"
