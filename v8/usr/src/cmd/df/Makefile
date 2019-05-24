CFLAGS = -Od2

df:	df.o
	cc -o df df.o

install:	df
	strip df
	mv df /bin
	chgrp sys /bin/df
	chmod g+s /bin/df

clean:
	rm -f df *.o
