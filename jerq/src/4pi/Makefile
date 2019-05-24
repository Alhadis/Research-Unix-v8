install:
	cd host; make
	cd term; make
	rm -f /usr/jerq/bin/pi /usr/jerq/bin/3pi
	cp host/4pi /usr/jerq/bin
	ln /usr/jerq/bin/4pi /usr/jerq/bin/pi
	ln /usr/jerq/bin/4pi /usr/jerq/bin/3pi
	cp term/pi.m /usr/jerq/mbin

clean:
	cd host; make clean
	cd term; make clean
