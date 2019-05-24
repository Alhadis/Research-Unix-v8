# NOTICE-NOT TO BE DISCLOSED OUTSIDE BELL SYS EXCEPT UNDER WRITTEN AGRMT
for i in $*
	do case $i in
		-ver) echo $0 version 2.0;exit;;
		-flags) echo $0 \[-flags\] \[-ver\];exit;;
		-*) echo unknown splitrules flag $i;exit;;
                *) echo splitrules takes no files;shift;continue;;
	esac
done
cat << !
     An infinitive is a verb form that contains the word "to."
Examples include:

 1.  to make

 2.  to pursue

 3.  to eat

 4.  to be going.

     An infinitive is said to be split when a word or phrase
occurs between "to" and the verb.  Possible split infinitives
include:

 1.  to often make

 2.  to quickly pursue

 3.  to immediately eat

 4.  to soon be going

     There is nothing ungrammatical about split infinitives; usu-
ally, however, they are awkward.  If the meaning of the phrase is
clear without the split infinitive, by all means don't use it.

     There are cases, however, where the meaning is not clear
unless the infinitive is split.  For example a and b do not mean
the same as c.

 a.  Really to understand calculus, you must do the exercises.

 b.  To understand calculus really, you must do the exercises.

 c.  To really understand calculus, you must do the exercises.

     In such cases, many grammarians will tell you it is accept-
able to use the split infinitive.  It is usually possible, how-
ever, to change the form of the sentence as in examples d and e,
and keep some readers from downgrading you.

 d.  Really understanding calculus, requires your doing the exer-
cises.

 e.  To understand calculus fully, you must do the exercises.


!
