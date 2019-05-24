CFLAGS = -Od2

passwd:	passwd.o
	cc -o passwd passwd.o

install:	passwd
	strip passwd
	mv passwd /bin
	chown root /bin/passwd
	chmod og-w /bin/passwd
	chmod u+s /bin/passwd

clean:
	rm -f passwd *.o
