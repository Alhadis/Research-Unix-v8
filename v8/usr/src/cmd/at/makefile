# This makefile must be run as super-user

SPOOL=/usr/spool/at

all:	at atrun

at:	at.c
	cc -Od2 -o at at.c

atrun:	atrun.c
	cc -Od2 -o atrun atrun.c

install:	all
	rm /bin/at /etc/atrun
	strip at atrun
	mv at /usr/bin
	chown root /usr/bin/at
	chmod u+s,og-w /usr/bin/at
	cp atrun /usr/lib
	chown bin /usr/lib/atrun

clean:
	rm -f at atrun

dirs:
	rm -fr $(SPOOL)
	mkdir $(SPOOL) $(SPOOL)/past
	chown root $(SPOOL) $(SPOOL)/past
	chmod og-w $(SPOOL) $(SPOOL)/past
	>$(SPOOL)/lasttimedone
	chmod og-w $(SPOOL)/lasttimedone
