ldpcs: ldpcs.o unpack.o
	cc -o ldpcs ldpcs.o unpack.o

clean:
	rm -f ldpcs.o unpack.o ldpcs

install: ldpcs
	strip ldpcs
	cp ldpcs /etc
