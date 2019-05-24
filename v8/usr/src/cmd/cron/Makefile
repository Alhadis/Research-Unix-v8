CFLAGS = -Od2
DEST = /etc/cron

cron:	cron.o
	cc -o cron cron.o

install:	cron
	strip cron
	mv cron $(DEST)
	chown root $(DEST)
	chmod og-x $(DEST)

clean:
	rm -f cron *.o
