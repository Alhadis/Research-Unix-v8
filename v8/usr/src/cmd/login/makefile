CFLAGS = -Od2

login:	login.o
	cc -o login login.o

install:	login
	strip login
	cp login /etc
	chmod og-wx /etc/login

clean:
	rm -f login.o login
