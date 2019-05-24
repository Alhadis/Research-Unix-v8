PROGS=tek hpplot 
all:	$(PROGS)

tek:	driver.o /usr/lib/lib4014.a
	cc -s -o tek  driver.o -l4014

hpplot:	driver.o /usr/lib/lib2621.a
	cc -s -o hpplot driver.o -l2621.a -lcurses -ltermlib


driver.o: driver.c
	cc -c -O driver.c

install:	all
	-cp $(PROGS) /usr/bin

clean:
	rm -f $(PROGS) *.o
