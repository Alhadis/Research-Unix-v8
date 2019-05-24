CFLAGS = -O

a.out:	mail.o
	cc mail.o

install:	a.out
	strip a.out
	rm -f /bin/mail
	cp a.out /bin/mail
	chown root /bin/mail
	chmod og-w,u+s /bin/mail
	rm -f /bin/rmail
	ln /bin/mail /bin/rmail

clean:
	rm -f a.out *.o
