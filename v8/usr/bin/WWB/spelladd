# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
trap 'rm -f /tmp/$$spadd; trap 0;exit' 0 1 2 3 15
for i in $*
	do case $i in
	-ver) echo $0 version 2.0;exit;;
	-flags) echo $0 \[-ver\] \[-flags\] word1 word2 ... wordn;exit;;
	-*) echo unknown spelladd flag $i;exit;;
	esac
done
if [ -f $HOME/lib ]
then echo "Spelladd can't make the directory $HOME/lib for you 
because you already have a file named $HOME/lib.

Change the name of the file to something else and run the program again."
	exit 1
else 	if [ ! -d $HOME/lib ]
	then echo spelladd is making a directory: $HOME/lib for you.
	mkdir $HOME/lib
	fi
fi
if test -r $HOME/lib/spelldict
then if test ! -w $HOME/lib/spelldict
	then echo Can\'t write on the file $HOME/lib/spelldict, check your permissions.
		exit 1
	fi
fi
echo $* >>$HOME/lib/spelldict
deroff -w $HOME/lib/spelldict | sort -u >/tmp/$$spadd
mv /tmp/$$spadd $HOME/lib/spelldict
