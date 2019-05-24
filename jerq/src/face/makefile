face: face.c
	cc -o face face.c
catalog.m:	catalog.c
	3cc -o catalog.m catalog.c
install:	face catalog.m
	cp face /usr/jerq/bin
	strip /usr/jerq/bin/face
	cp catalog.m /usr/jerq/mbin
clean:
	rm -f *.o catalog.m face core
