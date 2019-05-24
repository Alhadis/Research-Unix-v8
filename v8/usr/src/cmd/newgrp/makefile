D=/bin/newgrp

newgrp:	newgrp.o
	cc -o newgrp newgrp.o

install:	newgrp
	rm -f $D
	cp newgrp $D
	strip $D
	chown root $D
	chmod og-w,u+s $D

clean:
	rm -f newgrp *.o
