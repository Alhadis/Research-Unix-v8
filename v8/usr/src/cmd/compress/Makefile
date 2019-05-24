D = /usr/bin

compress:	compress.o
	cc -o compress compress.o

install:	compress
	rm -f $D/compress $D/uncompress $D/zcat
	cp compress $D
	strip $D/compress
	ln $D/compress $D/uncompress
	ln $D/uncompress $D/zcat
	chown bin,bin $D/compress
	chmod o-w,g+w $D/compress

clean:
	rm -f a.out *.o compress
