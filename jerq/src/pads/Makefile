pads.pri: pads.h
	/usr/jerq/bin/cleave pads
	touch pads.pri

install: pads.pri
	cd term; make
	cd host; make
	cp host/libpads.a /usr/jerq/lib
	ranlib	/usr/jerq/lib/libpads.a
	cp pads.pub /usr/jerq/include
	cp pads.pri /usr/jerq/include
	cp term/pads.m	/usr/jerq/mbin

clean:
	cd host; make clean
	cd term; make clean
	rm -f *junk* core jim.recover
