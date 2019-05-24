CFLAGS = -Od2

su:	su.o
	cc -o su su.o

install:	su
	strip su
	mv su /etc
	chown root /etc/su
	chmod og-w,u+s /etc/su

clean:
	rm -f su *.o
