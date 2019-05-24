
D = /bin/cp

a.out:	cp.c
	cc -Od2 cp.c

install:	a.out
	strip a.out
	rm -f $D
	cat a.out >$D
	chown bin $D
	chgrp bin $D
	chmod a+x,o-w,g+w $D

clean:
	rm -f *.o a.out
