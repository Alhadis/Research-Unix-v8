pr(){
	echo $#
}
add(){
	echo $*
}
mul(){
	multmp=""
	for i in $1
	do
		multmp="$multmp $2"
	done
	echo $multmp
}
n(){
	echo `seq $1 | sed 's/.*/1/'`
}
sub(){
	echo $1 | sed 's/'"$2"'//;s/^ *//'
}
factorial(){
	case "$*" in
	""|"1")	echo 1
		;;
	*)	mul "$*" "`factorial \`sub \"$*\" 1\``"
	esac
}
fact(){
	pr `factorial \`n $1\``
}
