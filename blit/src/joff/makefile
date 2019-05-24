wrong:
	@echo make install or make clean

install:
	cd host; make
	cd term; make
	cp host/a.out /usr/blit/lib/joff
	cp term/a.out /usr/blit/mbin/joff.m
	mstrip -gv /usr/blit/mbin/joff.m

clean:
	cd host; make clean
	cd term; make clean
