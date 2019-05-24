all:	notes.out owner.out manual.out index.out mans.out

notes.out:	notes.ms
	pic $T notes.ms | eqn $T | troff -ms $T > notes.out

owner.out:	dipsw.pic owner.ms
	pic $T owner.ms | troff -ms $T > owner.out

manual.out: mac manual manpages
	troff -ms $T mac manual manpages>manual.out

index.out:	ptxx
	troff $T ptx.mac > index.out

ptxx:	manpages
	grep "^\.Ja" manpages | sed 's/.Ja //' | sed 's/"//' | ptx -r -t -f > ptxx

mans.out:
	troff -man $T *.[1-9]* > mans.out
