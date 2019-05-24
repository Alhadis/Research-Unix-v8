CC=3CC
CFLAGS = -DMUX
OFILES = _new.o arith.o _main.o  _delete.o _handler.o _vec.o exit.o _ctor.o _dtor.o

libC.a: $(OFILES)
	3ar uv libC.a $(OFILES)
install:	libC.a
	cp libC.a /usr/jerq/lib
clean:
	rm -f core *.o libC.a
