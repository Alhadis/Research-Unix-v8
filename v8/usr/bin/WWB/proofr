# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
mflag=-ms
trap 'rm -f /tmp/$$*; trap 0; exit' 0 1 2  3 15
if test $# -eq 0
then echo Usage: proof\[e\]r \[-s\] filename ...
	exit 1
fi
for i in $*
	do case $i in
		-s) sflag=s; shift;continue;;
		-ver) echo $0 version 2.5::2.1,2.0; exit 0;;
		-flags) echo $0 \[-flags\] \[-ver\] \[file ...\];exit 0;;
		-*) echo unknown proof\[e\]r flag $i; exit 1;;
		*) if test ! -r $i
		   then echo Can\'t find the file $i\; try specifying a more complete pathname.; exit
		   fi
	esac
done
echo $*

if test $sflag
then
	echo -e "\n"--SPELLWWB for $*--
	if test -r $HOME/lib/spelldict
	then spell $*|sort|comm -23 - $HOME/lib/spelldict |pr -r -3 -t -i" "25
        else spell $*|pr -r -3 -t -i" "25
	fi

	echo -e "\n"--PUNCT and DOUBLE for $*--
	punct $*
	echo -e "\n"--DICTION for $*--
	if test -r $HOME/lib/ddict
	then	$L/dprog -l -f $HOME/lib/ddict $* 
	else	$L/dprog -l $* 
	fi
	echo -e "\n"--GRAM for $*--
	deroff -n $mflag  $*|$L/style1  |$L/style2 | $L/style3 -P -L| $L/gramlx 
	echo -e __"\n"
else

first=$1
echo -e "******************************  SPELLING  *******************************\n"
m=F
for i in $*
do echo -e "\n"
if test -r $HOME/lib/spelldict
then spell $i|sort|comm -23 - $HOME/lib/spelldict >/tmp/$$spell
else spell $i>/tmp/$$spell
fi
if test -s /tmp/$$spell
then echo -e Possible spelling errors in $i are:"\n"
	pr -3 -t -i" "25 /tmp/$$spell
	m=T
else	echo No spelling errors found in $i
fi
done
if test $m = T
then	echo -e '\nIf any of these words are spelled correctly, later type
                 spelladd word1 word2 ... wordn
to have them added to your spelldict file.'
fi
echo -e "\n\n*****************************  PUNCTUATION  *****************************\n"
m=F
for i in $*
do
echo -e "For file $i:\n"
sed "s/^'/./" $i >/tmp/$$in
if test $i = $first
then	echo -e '\nThe program next prints any sentence that it thinks is 
incorrectly punctuated and follows it by its correction.\n'
fi
$L/punlx </tmp/$$in >/tmp/$$punct
echo -e "\n"
bdiff /tmp/$$in /tmp/$$punct >/tmp/$$diff
if test ! -s /tmp/$$diff
then echo -e  No errors found in $i"\n"
else 	sed -f $L/seddiff /tmp/$$diff  
	m=T
	echo -e "\n\n"
fi
done
if test $m = T
then	echo -e For more information about punctuation rules, type:"\n                            " punctrules
fi
echo -e "\n\n*****************************  DOUBLE WORDS  ****************************"
for i in $*
do
echo -e "\nFor file $i:\n"
double $i
done
echo -e '\n\n*****************************  WORD CHOICE  *****************************\n
Sentences with possibly wordy or misused phrases are listed next, 
followed by suggested revisions.\n'
if test -r $HOME/lib/ddict
then echo -e NOTE: proofr is using your file $HOME/lib/ddict for additional phrases."\n"
fi
if test -r $HOME/lib/ddict
then   $L/dprog -l -f $HOME/lib/ddict  -o /tmp/$$dictsed $*
else    $L/dprog -l -o /tmp/$$dictsed $*
fi
if test -s /tmp/$$dictsed
then
	echo -e '\nPlease wait for the substitution phrases.

\n-------------------   Table of Substitutions   --------------------\n
PHRASE                     SUBSTITUTION\n'
	   sort -u /tmp/$$dictsed|sed -f $L/script.sed| split -50 -f /tmp/$$d_
	for i in /tmp/$$d_*
	do
		sed -n -f $i $L/suggest.d >>/tmp/$$d
	done
	sort /tmp/$$d
fi 
        echo -e '\n------------------------------------------------------------------\n
\n    * Not all the revisions will be appropriate for your document. 
    * When there is more than one suggestion for just one bracketed
      word, you will have to choose the case that fits your use.
    * Capitalized words are instructions, not suggestions.\n
NOTE: If you want this program to look for additional phrases
      or to stop looking for some, for instance to stop 
      flagging "impact," type the command dictadd.'
echo -e "\n\n**************************  GRAMMATICAL ERRORS  *************************"
m=F
for i in $*
do
echo -e "\nFor file $i:\n"
deroff -n $mflag $i | $L/style1 |$L/style2 | $L/style3 -P -L| $L/gramlx >/tmp/$$split
if test -s /tmp/$$split
then cat /tmp/$$split
	m=T
else echo  No grammer errors found 
fi
done
if test $m = T
then echo -e '\nFor information on split infinitives type:
                           splitrules'
fi
fi
