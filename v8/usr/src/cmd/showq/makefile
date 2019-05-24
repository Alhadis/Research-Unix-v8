D=/etc/showq
CFLAGS = -O

showq:	showq.o
	cc -o showq showq.o

install:	showq
	cp showq $D
	strip $D

clean:
	rm -f showq *.o
