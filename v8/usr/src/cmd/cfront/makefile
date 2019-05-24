munch:	munch.c
	CC -O -o munch munch.c

install:	munch
	cp munch /usr/bin; strip /usr/bin/munch
	cd cfront; make install
	cd libC; make install
clean:
	cd cfront; make clean
	cd libC; make clean
	rm -f cfront/*.[oi] munch.[oi] munch
