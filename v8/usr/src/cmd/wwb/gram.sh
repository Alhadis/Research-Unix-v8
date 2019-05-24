# new shell file for grammer checker(formerly splitinf)
L=/usr/lib/style
trap 'rm -f /tmp/$$* ; exit'  1 2 3 15
if test $# = 0
then
deroff -n -ms |$L/style1|$L/style2|$L/style3 -P -L|$L/gramlx
else
deroff -n -ms $*|$L/style1|$L/style2|$L/style3 -P -L|$L/gramlx
fi
