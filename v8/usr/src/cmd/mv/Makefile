D = /bin/mv

a.out:	mv.c
	cc -Od2 mv.c

install:	a.out
	strip a.out
	rm -f $D
	cp a.out $D
	chown root $D
	chgrp bin $D
	chmod og-w,u+s $D

clean:
	rm -f *.o a.out
