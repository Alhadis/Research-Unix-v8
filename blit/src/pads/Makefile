pads.pri: pads.h
	/usr/jerq/bin/cleave +E pads
	touch pads.pri

install:
	cp host/libpads.a /usr/blit/lib
	ranlib	/usr/blit/lib/libpads.a
	cp pads.pub /usr/blit/include
	cp pads.pri /usr/blit/include
	cp term/pads.m	/usr/blit/mbin

clean:
	rm -f *junk* core jim.recover
