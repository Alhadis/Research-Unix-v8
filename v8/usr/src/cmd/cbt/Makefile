all:    btbuild btcat btcreat btran btreport btsquash

btbuild:	btbuild.c
	cc btbuild.c -lcbt -o btbuild

btcat:	btcat.c
	cc btcat.c -lcbt -o btcat

btcreat:	btcreat.c
	cc btcreat.c -lcbt -o btcreat

btdelete:	btdelete.c
	cc btdelete.c -lcbt -o btdelete

btran:	btran.c
	cc btran.c -lcbt -o btran

btreport:	btreport.c
	cc btreport.c -lcbt -o btreport

btsquash:	btsquash.c
	cc btsquash.c -lcbt -o btsquash

install:
	strip btbuild btcat btcreat btdelete btran btreport btsquash
	mv btbuild btcat btcreat btdelete btran btreport btsquash /usr/lib/btree

clean:
	rm -f btbuild btcat btcreat btran btreport btsquash
