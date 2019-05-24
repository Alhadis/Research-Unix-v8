#! /bin/sh
# V, X flags, F files, C commands for auxiliary list and history
# D_SPELL dictionary, H_SPELL history, S_SPELL stoplist, P_SPELL program
# V_SPELL -v file, A_SPELL auxiliary list

F= V= X= C="sort -u +0f +0"
P_SPELL=${P_SPELL-/usr/lib/spell}

for A
do
	case $A in
	-v)	V_SPELL=${V_SPELL-/tmp/spell$$} 
		trap "rm -f $V_SPELL" 0 1 2 13 15
		V="-v" ;;
	-b) 	D_SPELL=${D_SPELL-/usr/dict/hlistb}
		X="$X -b" ;;
#	+?*)	if test ! -r ${A_SPELL=`expr $A : '+\(.*\)'`}	#WWB compatibility
#		then echo spell: cannot open $A_SPELL; exit 1
#		fi ;;
	-x)	X="$X -x" ;;
	*)	F="$F $A"
	esac
done

if test -w ${H_SPELL=/usr/dict/spellhist}
then C="$C | tee -a $H_SPELL"
fi
if test -r ${A_SPELL=$HOME/lib/spelldict}
then C="$C | fgrep -x -v -f $A_SPELL"
fi

deroff -w $F |
 $P_SPELL ${S_SPELL-/usr/dict/hstop} 1 $X |
 $P_SPELL ${D_SPELL-/usr/dict/hlista} ${V_SPELL=/dev/null} $X $V |
 eval $C

case $V in
*-v*)	sort -u +1f +1 $V_SPELL
esac
