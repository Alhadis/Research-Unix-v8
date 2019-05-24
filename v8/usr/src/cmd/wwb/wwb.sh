# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
L=/usr/lib/style
trap 'rm -f /tmp/$$*; trap 0; exit' 0 1 2 3 15
if test $# = 0
then echo "Usage: wwb [-mm|-ms] [-li|+li] [-s] [-c|-t|-tm] [-x standards-file] file ..."
	exit 1
fi
mflag=-ms
mlflag=-ml
sflag=
fflag=
file=
xfile=
audflag=m
while test '$1'
	do case $1 in
		-ver) echo $0 version 2.5:: 2.1,2.0; exit 0;;
		-flags) echo "$0 [-flags] [-ver] [-mm|-ms] [-li|+li] [-tm|-t|-c|-x standards-file] [-s]  [file ...]";exit 0;;
		-mm) mflag=-mm;shift;continue;;
		-ms) mflag=-ms; shift;continue;;
		-li|-ml) mlflag=-ml;shift;continue;;
		+li) mlflag=;shift;continue;;
		-f) fflag=F; shift; file=$1;shift;continue;;
		-c) audflag=c;shift;continue;;
		-t) audflag=t;shift;continue;;
		-tm) audflag=m;shift;continue;;
		-x) audflag=x;shift; xfile=$1; shift;continue;;
		-s) sflag=s;shift;continue;;
		-*) echo unknown wwb flag $1; exit;;
		*) break;;
	esac
done
if test $fflag
then	if test ! -r $file
	then echo Can\'t find the file $file, try specifying a more complete pathname.
	exit 1
	fi
fi
if test $audflag = x
then	if test ! -r $xfile
	then echo Can\'t find the standards file $xfile, try specifying a more complete pathname.
	exit 1
	fi
fi
if test $# = 0
then 	echo "Usage: wwb [-mm|-ms] [-li|+li] [-s] [-c|-t|-tm] [-x standards-file] file ..."
	exit 1
fi
for i in $*
do
 	if test ! -r $i
	then echo Can\'t find the file $i\; try specifying a more complete pathname.
	exit 1
	fi
	if test $sflag
	then
        echo -e "\n"--SPELLWWB for $i--
		if test -r $HOME/lib/spelldict
		then spell $i|sort|comm -23 - $HOME/lib/spelldict |pr -r -3 -t -i" "25
		else spell $i |pr -r -3 -t -i" "25
		fi
        echo -e "\n"--PUNCT for $i--
	sed "s/^'/./" $i >/tmp/$$in
		$L/punlx < /tmp/$$in| bdiff /tmp/$$in - |sed -f $L/seddiff 
        echo -e "\n"--DOUBLE for $i--
		double $i
        echo -e "\n"--DICTION for $i--
		if test -r $HOME/lib/ddict
		then	$L/dprog -l -f $HOME/lib/ddict $i 
		else	$L/dprog -l $i 
		fi
        echo -e "\n"--GRAM for $i--
		deroff -n $mflag $mlflag  $i|$L/style1  |$L/style2 | tee /tmp/$$style2 | $L/style3 -P -L| $L/gramlx 
		echo -e __"\n"
else
{ 
echo -e "******************************  SPELLING  *******************************\n"
if test -r $HOME/lib/spelldict
then spell $i|sort|comm -23 - $HOME/lib/spelldict >/tmp/$$spell
else spell $i>/tmp/$$spell
fi
if test -s /tmp/$$spell
then echo -e Possible spelling errors in $i are:"\n"
	pr -3 -t -i" "25 /tmp/$$spell
	echo -e '\nIf any of these words are spelled correctly, later type
                  spelladd word1 word2 ... wordn
to have them added to your spelldict file.'
else	echo No spelling errors found in $i
fi
echo -e "\n\n*****************************  PUNCTUATION  *****************************\n"
echo -e "For file $i:\n"
sed "s/^'/./" $i >/tmp/$$in
	echo -e '\nThe program next prints any sentence that it thinks is 
incorrectly punctuated and follows it by its correction.\n'
$L/punlx </tmp/$$in >/tmp/$$punct
echo -e "\n"
bdiff  /tmp/$$in /tmp/$$punct >/tmp/$$diff
if test ! -s /tmp/$$diff
then echo -e  No errors found in $i"\n"
else	sed -f $L/seddiff /tmp/$$diff
	echo -e "\n\n"
	echo -e For more information about punctuation rules, type:"\n                            " punctrules
fi
echo -e "\n\n*****************************  DOUBLE WORDS  ****************************"
echo -e "\nFor file $i:\n"
double $i
echo -e "\n\n*****************************  WORD CHOICE  *****************************\n"
echo -e 'Sentences with possibly wordy or misused phrases are listed next, 
followed by suggested revisions.\n'
if test -r $HOME/lib/ddict
then echo -e NOTE: proofr is using your file $HOME/lib/ddict for additional phrases."\n"
fi
echo -e "\nFor file $i\n"
if test -r $HOME/lib/ddict
then	$L/dprog -l -f $HOME/lib/ddict -o /tmp/$$dictsed $i
else $L/dprog -l -o /tmp/$$dictsed $i
fi
if test -s /tmp/$$dictsed 
then
 	echo -e '\nPlease wait for the substitution phrases\n
\n-------------------   Table of Substitutions   --------------------\n
PHRASE                     SUBSTITUTION\n'
 	 sort -u /tmp/$$dictsed |sed -f $L/script.sed | split -50 -f /tmp/$$d_
	for j in /tmp/$$d_*
	do
		sed -n -f $j $L/suggest.d >>/tmp/$$d
	done
	sort /tmp/$$d
	echo -e '\n------------------------------------------------------------------\n
\n    * Not all the revisions will be appropriate for your document. 
    * When there is more than one suggestion for just one bracketed
      word, you will have to choose the case that fits your use.
    * Capitalized words are instructions, not suggestions.\n
NOTE: If you want this program to look for additional phrases
      or to stop looking for some, for instance to stop 
      flagging "impact," type the command dictadd.'
fi
echo -e "\n\n***************************  GRAMMATICAL ERRORS  ************************"
echo -e "\nFor file $i:\n"
deroff -n $mflag $mlflag  $i|$L/style1  |$L/style2 | tee /tmp/$$style2 | $L/style3 -P -L| $L/gramlx >/tmp/$$split
if test -s /tmp/$$split
then cat /tmp/$$split
	echo -e '\nFor information on split infinitives type:
	                           splitrules'
else echo  No split infinitives found 
fi; }|pr -h "PROOFR OUTPUT FOR $i"
fi
		if test $fflag
		then	if test $sflag
			then $L/prose f${sflag}${audflag} $xfile  < $file
			else $L/prose f${audflag} $xfile < $file |pr -e0 -h "PROSE OUTPUT FOR $file"
			fi
		else
			if test $sflag
			then  $L/style3 < /tmp/$$style2 | tee styl.tmp| $L/prose ${sflag}${audflag} $xfile  $$
			else $L/style3 < /tmp/$$style2 | tee styl.tmp| $L/prose ${audflag} $xfile $$ |pr -h "PROSE OUTPUT FOR $i"
			fi
		fi
done
