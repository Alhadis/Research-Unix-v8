# F FEATUREs, M map files, A other arguments
FEATURE=no
MAPPROG=${MAPPROG-/usr/lib/map}
F= M= A=
for i 
do
	case $FEATURE in
	no)
		case $i in
		-f)
			FEATURE=yes 
			F="$F " ;;
		*)
			A="$A $i"
		esac ;;
	yes)
		case $i in
		-f)
			: ;;
		-*)
			A="$A $i"
			FEATURE=no ;;
		riv*2)
			F="$F 201 202";;
		riv*3)
			F="$F 201 202 203";;
		riv*4)
			F="$F 201 202 203 204";;
		riv*)
			F="$F 201";;
		iriv*2)
			F="$F 206 207";;
		iriv*[34])
			F="$F 206 207 208";;
		iriv*)
			F="$F 206";;
		coast*2|shore*2)
			F="$F 102";;
		coast*3|shore*3)
			F="$F 102 103";;
		coast*4|shore*4)
			F="$F 102 103 104";;
		coast*|shore*)
			;;
		ilake*[234]|ishore*[234])
			F="$F 106 107";;
		ilake*|ishore*)
			F="$F 106";;
		reef*)
			F="$F 108";;
		canal*2)
			F="$F 210 211";;
		canal*[34])
			F="$F 210 211 212";;
		canal*)
			F="$F 210";;
		glacier*)
			F="$F 115";;
		state*|province*)
			F="$F 401";;
		countr*2)
			F="$F 301 302";;
		countr*[34])
			F="$F 301 302 303";;
		countr*)
			F="$F 301";;
		salt*[234])
			F="$F 109 110";;
		salt*)
			F="$F 109";;
		ice*[234]|shel*[234])
			F="$F 113 114";;
		ice*|shel*)
			F="$F 113";;
		*)
			echo map: unknown feature $i 2>&1
			exit 1
		esac
	esac
done
for j in $F
do
	if test -r /usr1/maps/$j
		then M="$M /usr1/maps/$j"
	fi
done

case $F in
"")	: ;;
*)	if test -r /usr1/maps/101
		then M="101 $M"
	fi
	M="-m $M"
esac

MAP=${MAP-world} MAPDIR=${MAPDIR-/usr/dict} $MAPPROG $A $M
